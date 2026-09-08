//! Times the sparse solvers that could replace CHOLMOD in safelanes.c.
//!
//! safelanes builds one stiffness matrix over the whole jump network and
//! solves it against a load for every lane-anchoring spob. Each backend here
//! does a full turn of that, and is checked against CHOLMOD's answer as well
//! as its clock. The readme covers producing an input.

#![allow(non_upper_case_globals, non_camel_case_types, non_snake_case)]

use std::{path::Path, time::Instant};

// bindgen emits the whole CHOLMOD API, of which this uses a fraction.
#[allow(clippy::all, dead_code, unnecessary_transmutes)]
mod cholmod {
   include!(concat!(env!("OUT_DIR"), "/cholmod.rs"));
}

/// A backend's result, kept so answers can be compared and not just times.
struct Solved {
   factor_ms: f64,
   solve_ms: f64,
   /// Column major, as every backend here stores it.
   x: Vec<f64>,
}

/// The equations safelanes solves.
struct System {
   n: usize,
   /// Upper triangle, once folded: (row, column, value).
   stiff: Vec<(u32, u32, f64)>,
   /// Column major, `n` rows by `rhs_cols` columns.
   ftilde: Vec<f64>,
   rhs_cols: usize,
   qtq: Vec<(u32, u32, f64)>,
}

/// Adds together entries that land on the same place, optionally moving
/// anything below the diagonal above it first.
///
/// The stiffness matrix reaches CHOLMOD marked upper triangular, which means
/// entries below the diagonal get moved up rather than dropped. Doing the same
/// here hands every backend the matrix CHOLMOD would have built.
fn sum_duplicates(entries: Vec<(u32, u32, f64)>, fold: bool) -> Vec<(u32, u32, f64)> {
   let mut merged: std::collections::HashMap<(u32, u32), f64> =
      std::collections::HashMap::with_capacity(entries.len());
   for (i, j, v) in entries {
      let key = if fold && i > j { (j, i) } else { (i, j) };
      *merged.entry(key).or_insert(0.0) += v;
   }
   let mut out: Vec<_> = merged.into_iter().map(|((i, j), v)| (i, j, v)).collect();
   out.sort_unstable_by_key(|(i, j, _)| (*j, *i));
   out
}

fn read(path: &Path) -> System {
   let text = std::fs::read_to_string(path)
      .unwrap_or_else(|err| panic!("reading {}: {err}", path.display()));
   let mut lines = text.lines();

   let header = |line: Option<&str>| -> Vec<usize> {
      line
         .expect("the dump ends early")
         .split_whitespace()
         .skip(1)
         .map(|f| f.parse().expect("dimensions are integers"))
         .collect()
   };

   let dims = header(lines.next());
   let (n, nnz) = (dims[0], dims[2]);
   let triplets = |lines: &mut std::str::Lines, count: usize| -> Vec<(u32, u32, f64)> {
      (0..count)
         .map(|_| {
            let line = lines.next().expect("the dump ends early");
            let mut f = line.split_whitespace();
            let mut next = || f.next().expect("a triplet has three fields");
            (
               next().parse().expect("indices are integers"),
               next().parse().expect("indices are integers"),
               next().parse().expect("values are floats"),
            )
         })
         .collect()
   };
   // One entry per edge, so the same place is written to repeatedly.
   let stiff = sum_duplicates(triplets(&mut lines, nnz), true);

   let dims = header(lines.next());
   let (rows, rhs_cols) = (dims[0], dims[1]);
   let mut ftilde = vec![0.0; rows * rhs_cols];
   for (i, j, v) in triplets(&mut lines, dims[2]) {
      ftilde[j as usize * rows + i as usize] = v;
   }

   let dims = header(lines.next());
   // QtQ comes out of cholmod_aat whole, so it keeps both triangles.
   let qtq = sum_duplicates(triplets(&mut lines, dims[2]), false);

   System {
      n,
      stiff,
      ftilde,
      rhs_cols,
      qtq,
   }
}

/// CHOLMOD, which is what safelanes uses today and the answer to match.
///
/// This is one turn of safelanes: factor the stiffness matrix, solve for the
/// potentials, multiply through QtQ, then solve again for the gradient that
/// decides which lanes get built.
fn solve_cholmod(s: &System) -> Solved {
   unsafe {
      let mut c = std::mem::zeroed::<cholmod::cholmod_common>();
      cholmod::cholmod_start(&mut c);

      let mut trip = cholmod::cholmod_allocate_triplet(
         s.n,
         s.n,
         s.stiff.len(),
         1, // upper triangular, as safelanes stores it
         cholmod::CHOLMOD_REAL as i32,
         &mut c,
      );
      let ti = (*trip).i as *mut i32;
      let tj = (*trip).j as *mut i32;
      let tx = (*trip).x as *mut f64;
      for (k, (i, j, v)) in s.stiff.iter().enumerate() {
         *ti.add(k) = *i as i32;
         *tj.add(k) = *j as i32;
         *tx.add(k) = *v;
      }
      (*trip).nnz = s.stiff.len();

      let start = Instant::now();
      let mut a = cholmod::cholmod_triplet_to_sparse(trip, 0, &mut c);
      let mut factor = cholmod::cholmod_analyze(a, &mut c);
      cholmod::cholmod_factorize(a, factor, &mut c);
      let factor_ms = start.elapsed().as_secs_f64() * 1e3;

      let mut b = cholmod::cholmod_allocate_dense(
         s.n,
         s.rhs_cols,
         s.n,
         cholmod::CHOLMOD_REAL as i32,
         &mut c,
      );
      let bx = (*b).x as *mut f64;
      for (k, v) in s.ftilde.iter().enumerate() {
         *bx.add(k) = *v;
      }

      // QtQ is stored whole rather than as a triangle, so it goes in unsymmetric.
      let mut qtrip = cholmod::cholmod_allocate_triplet(
         s.n,
         s.n,
         s.qtq.len(),
         0,
         cholmod::CHOLMOD_REAL as i32,
         &mut c,
      );
      let qi = (*qtrip).i as *mut i32;
      let qj = (*qtrip).j as *mut i32;
      let qx = (*qtrip).x as *mut f64;
      for (k, (i, j, v)) in s.qtq.iter().enumerate() {
         *qi.add(k) = *i as i32;
         *qj.add(k) = *j as i32;
         *qx.add(k) = *v;
      }
      (*qtrip).nnz = s.qtq.len();
      let mut qtq = cholmod::cholmod_triplet_to_sparse(qtrip, 0, &mut c);

      // Solve, multiply through QtQ, solve again: one turn's worth.
      let start = Instant::now();
      let mut u = cholmod::cholmod_solve(cholmod::CHOLMOD_A as i32, factor, b, &mut c);
      let mut prod = cholmod::cholmod_zeros(s.n, s.rhs_cols, cholmod::CHOLMOD_REAL as i32, &mut c);
      let mut neg_one = [-1.0f64, 0.0];
      let mut zero = [0.0f64, 0.0];
      cholmod::cholmod_sdmult(
         qtq,
         0,
         neg_one.as_mut_ptr(),
         zero.as_mut_ptr(),
         u,
         prod,
         &mut c,
      );
      let mut x = cholmod::cholmod_solve(cholmod::CHOLMOD_A as i32, factor, prod, &mut c);
      let solve_ms = start.elapsed().as_secs_f64() * 1e3;

      let xs = std::slice::from_raw_parts((*x).x as *const f64, s.n * s.rhs_cols).to_vec();
      cholmod::cholmod_free_dense(&mut x, &mut c);
      cholmod::cholmod_free_dense(&mut prod, &mut c);
      cholmod::cholmod_free_dense(&mut u, &mut c);
      cholmod::cholmod_free_sparse(&mut qtq, &mut c);
      cholmod::cholmod_free_triplet(&mut qtrip, &mut c);
      cholmod::cholmod_free_dense(&mut b, &mut c);
      cholmod::cholmod_free_factor(&mut factor, &mut c);
      cholmod::cholmod_free_sparse(&mut a, &mut c);
      cholmod::cholmod_free_triplet(&mut trip, &mut c);
      cholmod::cholmod_finish(&mut c);
      Solved {
         factor_ms,
         solve_ms,
         x: xs,
      }
   }
}

/// faer's sparse Cholesky, which brings its own AMD ordering.
fn solve_faer(s: &System) -> Solved {
   use faer::{Mat, Side, linalg::solvers::Solve, sparse::SparseColMat};

   let triplets: Vec<_> = s
      .stiff
      .iter()
      .map(|(i, j, v)| faer::sparse::Triplet::new(*i as usize, *j as usize, *v))
      .collect();
   let a = SparseColMat::<usize, f64>::try_new_from_triplets(s.n, s.n, &triplets)
      .expect("the dumped triplets describe a valid matrix");
   let qtq_triplets: Vec<_> = s
      .qtq
      .iter()
      .map(|(i, j, v)| faer::sparse::Triplet::new(*i as usize, *j as usize, *v))
      .collect();
   let qtq = SparseColMat::<usize, f64>::try_new_from_triplets(s.n, s.n, &qtq_triplets)
      .expect("the dumped triplets describe a valid matrix");

   let start = Instant::now();
   let llt = a
      .sp_cholesky(Side::Upper)
      .expect("the stiffness matrix is positive definite");
   let factor_ms = start.elapsed().as_secs_f64() * 1e3;

   let rhs = Mat::from_fn(s.n, s.rhs_cols, |i, j| s.ftilde[j * s.n + i]);
   let start = Instant::now();
   let u = llt.solve(&rhs);
   let x = llt.solve(&(-(&qtq * &u)));
   let solve_ms = start.elapsed().as_secs_f64() * 1e3;

   Solved {
      factor_ms,
      solve_ms,
      x: (0..s.rhs_cols)
         .flat_map(|j| (0..s.n).map(move |i| (i, j)))
         .map(|(i, j)| x[(i, j)])
         .collect(),
   }
}

/// nalgebra-sparse, whose Cholesky documents that it does no fill reduction.
fn solve_nalgebra(s: &System) -> Solved {
   use nalgebra_sparse::{CooMatrix, CscMatrix, factorization::CscCholesky};

   // Its Cholesky wants the whole matrix, not one triangle.
   let mut coo = CooMatrix::new(s.n, s.n);
   for (i, j, v) in &s.stiff {
      coo.push(*i as usize, *j as usize, *v);
      if i != j {
         coo.push(*j as usize, *i as usize, *v);
      }
   }
   let a = CscMatrix::from(&coo);

   let mut qcoo = CooMatrix::new(s.n, s.n);
   for (i, j, v) in &s.qtq {
      qcoo.push(*i as usize, *j as usize, *v);
   }
   let qtq = CscMatrix::from(&qcoo);

   let start = Instant::now();
   let chol = match CscCholesky::factor(&a) {
      Ok(chol) => chol,
      Err(_) => {
         return Solved {
            factor_ms: f64::NAN,
            solve_ms: f64::NAN,
            x: Vec::new(),
         };
      }
   };
   let factor_ms = start.elapsed().as_secs_f64() * 1e3;

   let rhs = nalgebra::DMatrix::from_fn(s.n, s.rhs_cols, |i, j| s.ftilde[j * s.n + i]);
   let start = Instant::now();
   let u = chol.solve(rhs.as_view());
   let prod = -(&qtq * &u);
   let x = chol.solve(prod.as_view());
   let solve_ms = start.elapsed().as_secs_f64() * 1e3;

   Solved {
      factor_ms,
      solve_ms,
      x: x.as_slice().to_vec(),
   }
}

fn main() {
   let path = std::env::args().nth(1).unwrap_or_else(|| {
      Path::new(env!("CARGO_MANIFEST_DIR"))
         .join("problems/universe.txt")
         .to_string_lossy()
         .into_owned()
   });
   let s = read(Path::new(&path));
   println!(
      "  system: {}x{}, {} nonzeros upper triangle, {} right hand sides, QtQ {} nonzeros",
      s.n,
      s.n,
      s.stiff.len(),
      s.rhs_cols,
      s.qtq.len()
   );

   let mut reference: Option<Vec<f64>> = None;
   for (name, run) in [
      ("CHOLMOD ", solve_cholmod as fn(&System) -> Solved),
      ("faer    ", solve_faer),
      ("nalgebra", solve_nalgebra),
   ] {
      let got = run(&s);
      let agreement = match &reference {
         None => {
            reference = Some(got.x.clone());
            String::from("reference")
         }
         Some(want) if got.x.len() == want.len() => {
            // Relative L2 over the whole solution. A per-element relative
            // error is not meaningful here: the values span many orders of
            // magnitude and the ones near zero make it blow up regardless of
            // how good the solve was.
            let (diff, norm) = want.iter().zip(&got.x).fold((0.0, 0.0), |(d, n), (a, b)| {
               (d + (a - b) * (a - b), n + a * a)
            });
            let worst = want
               .iter()
               .zip(&got.x)
               .map(|(a, b)| (a - b).abs())
               .fold(0.0f64, f64::max);
            format!(
               "relative L2 {:.2e}, largest difference {worst:.2e}",
               diff.sqrt() / norm.sqrt()
            )
         }
         Some(_) => String::from("did not solve"),
      };
      println!(
         "  {name}  factor {:8.2} ms   solve {:8.2} ms   total {:8.2} ms   {agreement}",
         got.factor_ms,
         got.solve_ms,
         got.factor_ms + got.solve_ms
      );
   }
}
