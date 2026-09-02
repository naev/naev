//! Times linear programming backends on problems equipopt actually produced.
//!
//! nlua_linopt drives glpk. This compares it against the candidates for
//! replacing it on the same problems, read from the MPS files
//! `linopt:write_problem` writes. See the readme for collecting more.

#![allow(non_upper_case_globals, non_camel_case_types, non_snake_case)]

use std::{ffi::CString, path::Path, time::Instant};

// bindgen emits the whole glpk API, of which this uses a fraction, and does
// not write to anyone's lint standards.
#[allow(clippy::all, dead_code)]
mod glpk {
   include!(concat!(env!("OUT_DIR"), "/glpk.rs"));
}

/// The budget nlua_linopt gives glpk, in milliseconds.
const TIME_LIMIT_MS: i32 = 1000;

/// The heuristics HiGHS runs looking for a feasible solution. equipopt's
/// problems are solved by presolve before any of them help, so they are pure
/// overhead here.
const HIGHS_HEURISTICS: &[&str] = &[
   "mip_heuristic_run_feasibility_jump",
   "mip_heuristic_run_rins",
   "mip_heuristic_run_rens",
   "mip_heuristic_run_root_reduced_cost",
   "mip_heuristic_run_shifting",
   "mip_heuristic_run_zi_round",
];

/// A problem in the triplet form the Lua API builds.
struct Problem {
   ncols: usize,
   nrows: usize,
   /// Objective coefficients. equipopt maximises.
   obj: Vec<f64>,
   /// Column bounds, and whether the column is integral.
   cols: Vec<(f64, f64, bool)>,
   /// Row bounds, either side absent as in `set_row`.
   rows: Vec<(Option<f64>, Option<f64>)>,
   /// Zero-based (row, column, value).
   entries: Vec<(usize, usize, f64)>,
}

/// Reads an MPS file through glpk and hands back both the loaded problem and a
/// copy every backend can be fed from, since not all of them read MPS.
fn read(path: &Path) -> (*mut glpk::glp_prob, Problem) {
   let c_path = CString::new(path.to_string_lossy().as_bytes()).expect("paths hold no nul bytes");
   unsafe {
      // glpk narrates the read on stdout otherwise.
      glpk::glp_term_out(glpk::GLP_OFF as i32);
      let lp = glpk::glp_create_prob();
      let rc = glpk::glp_read_mps(
         lp,
         glpk::GLP_MPS_FILE as i32,
         std::ptr::null(),
         c_path.as_ptr(),
      );
      assert_eq!(rc, 0, "glpk could not read {}", path.display());
      // Free MPS carries no objective sense and equipopt maximises, so reading
      // one back without this solves a different problem.
      glpk::glp_set_obj_dir(lp, glpk::GLP_MAX as i32);

      let ncols = glpk::glp_get_num_cols(lp) as usize;
      let nrows = glpk::glp_get_num_rows(lp) as usize;
      let obj = (1..=ncols)
         .map(|j| glpk::glp_get_obj_coef(lp, j as i32))
         .collect();
      let cols = (1..=ncols)
         .map(|j| {
            let j = j as i32;
            (
               glpk::glp_get_col_lb(lp, j),
               glpk::glp_get_col_ub(lp, j),
               glpk::glp_get_col_kind(lp, j) != glpk::GLP_CV as i32,
            )
         })
         .collect();

      let mut rows = Vec::with_capacity(nrows);
      let mut entries = Vec::new();
      let mut ind = vec![0i32; ncols + 1];
      let mut val = vec![0f64; ncols + 1];
      for i in 1..=nrows {
         let i = i as i32;
         rows.push(match glpk::glp_get_row_type(lp, i) as u32 {
            glpk::GLP_FR => (None, None),
            glpk::GLP_LO => (Some(glpk::glp_get_row_lb(lp, i)), None),
            glpk::GLP_UP => (None, Some(glpk::glp_get_row_ub(lp, i))),
            _ => (
               Some(glpk::glp_get_row_lb(lp, i)),
               Some(glpk::glp_get_row_ub(lp, i)),
            ),
         });
         let len = glpk::glp_get_mat_row(lp, i, ind.as_mut_ptr(), val.as_mut_ptr()) as usize;
         for k in 1..=len {
            entries.push((i as usize - 1, ind[k] as usize - 1, val[k]));
         }
      }

      (
         lp,
         Problem {
            ncols,
            nrows,
            obj,
            cols,
            rows,
            entries,
         },
      )
   }
}

fn solve_glpk(lp: *mut glpk::glp_prob) -> (f64, f64) {
   unsafe {
      let start = Instant::now();
      let mut smcp = std::mem::zeroed::<glpk::glp_smcp>();
      glpk::glp_init_smcp(&mut smcp);
      smcp.msg_lev = glpk::GLP_MSG_ERR as i32;
      smcp.tm_lim = TIME_LIMIT_MS;
      glpk::glp_simplex(lp, &smcp);

      let mut iocp = std::mem::zeroed::<glpk::glp_iocp>();
      glpk::glp_init_iocp(&mut iocp);
      iocp.msg_lev = glpk::GLP_MSG_ERR as i32;
      iocp.tm_lim = TIME_LIMIT_MS;
      iocp.presolve = glpk::GLP_ON as i32;
      glpk::glp_intopt(lp, &iocp);

      let ms = start.elapsed().as_secs_f64() * 1e3;
      (ms, glpk::glp_mip_obj_val(lp))
   }
}

/// Builds the problem through good_lp and hands it to a backend. Both backends
/// go through the same path, since good_lp is the API a port would use.
fn solve<M: good_lp::SolverModel>(
   p: &Problem,
   backend: impl FnMut(good_lp::variable::UnsolvedProblem) -> M,
   configure: impl FnOnce(M) -> M,
) -> (f64, f64) {
   use good_lp::{Expression, ProblemVariables, Solution, variable};

   let start = Instant::now();
   let mut vars = ProblemVariables::new();
   let cols: Vec<_> = p
      .cols
      .iter()
      .map(|(lo, hi, integral)| {
         let v = variable().min(*lo).max(*hi);
         vars.add(if *integral { v.integer() } else { v })
      })
      .collect();

   let objective: Expression = cols.iter().enumerate().map(|(c, v)| p.obj[c] * *v).sum();
   let mut per_row: Vec<Expression> = vec![Expression::from(0.0); p.nrows];
   for (r, c, v) in &p.entries {
      per_row[*r] += *v * cols[*c];
   }

   let mut model = configure(vars.maximise(objective).using(backend));
   for (r, (lo, hi)) in p.rows.iter().enumerate() {
      let row = per_row[r].clone();
      if let Some(lo) = lo {
         model.add_constraint(row.clone().geq(*lo));
      }
      if let Some(hi) = hi {
         model.add_constraint(row.leq(*hi));
      }
   }

   let z = match model.solve() {
      Ok(solution) => cols
         .iter()
         .enumerate()
         .map(|(c, v)| p.obj[c] * solution.value(*v))
         .sum(),
      // Reported rather than fatal: how a backend handles a problem it cannot
      // solve is part of what is being compared.
      Err(_) => f64::NAN,
   };
   (start.elapsed().as_secs_f64() * 1e3, z)
}

fn main() {
   let args: Vec<String> = std::env::args().skip(1).collect();
   let files: Vec<std::path::PathBuf> = if args.is_empty() {
      let dir = Path::new(env!("CARGO_MANIFEST_DIR")).join("problems");
      let mut found: Vec<_> = std::fs::read_dir(&dir)
         .unwrap_or_else(|err| panic!("reading {}: {err}", dir.display()))
         .filter_map(|entry| entry.ok().map(|e| e.path()))
         .filter(|p| p.extension().is_some_and(|e| e == "mps"))
         .collect();
      found.sort();
      found
   } else {
      args.iter().map(std::path::PathBuf::from).collect()
   };

   println!(
      "{:>5} {:>5} {:>9} {:>10} {:>10} {:>9}   problem",
      "cols", "rows", "glpk", "HiGHS", "HiGHS+", "microlp"
   );

   let mut totals = [0.0f64; 4];
   let mut disagreed = 0;
   for path in &files {
      let (lp, problem) = read(path);
      let (glpk_ms, zg) = solve_glpk(lp);
      unsafe { glpk::glp_delete_prob(lp) };
      let limit = f64::from(TIME_LIMIT_MS) / 1e3;
      let (highs_ms, zh) = solve(&problem, good_lp::highs, |m| m.set_time_limit(limit));
      let (tuned_ms, zt) = solve(&problem, good_lp::highs, |m| {
         let mut m = m.set_time_limit(limit);
         for name in HIGHS_HEURISTICS {
            m = m.set_option(*name, false);
         }
         m
      });
      let (micro_ms, zm) = solve(&problem, good_lp::microlp, |m| m);

      let agrees = |z: f64| (zg - z).abs() <= 1e-6 * zg.abs().max(1.0);
      let mark = |z: f64| if agrees(z) { " " } else { "*" };
      if !agrees(zh) || !agrees(zt) || !agrees(zm) {
         disagreed += 1;
      }

      println!(
         "{:>5} {:>5} {:>9.2} {:>9.2}{} {:>9.2}{} {:>8.2}{}  {}",
         problem.ncols,
         problem.nrows,
         glpk_ms,
         highs_ms,
         mark(zh),
         tuned_ms,
         mark(zt),
         micro_ms,
         mark(zm),
         path.file_name().unwrap_or_default().to_string_lossy()
      );
      for (slot, ms) in [glpk_ms, highs_ms, tuned_ms, micro_ms].iter().enumerate() {
         totals[slot] += ms;
      }
   }

   let n = files.len().max(1) as f64;
   println!(
      "\n{:>5} {:>5} {:>9.2} {:>10.2} {:>10.2} {:>9.2}   average over {} problems",
      "",
      "",
      totals[0] / n,
      totals[1] / n,
      totals[2] / n,
      totals[3] / n,
      files.len()
   );
   println!(
      "  HiGHS+ is HiGHS with its feasibility heuristics off. * marks a result \
       that differs from glpk's; {disagreed} problem(s) disagreed."
   );
}
