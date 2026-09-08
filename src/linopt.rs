use anyhow::Result;
use mlua::{BorrowedStr, Table, UserData, UserDataMethods};
use std::ffi::CString;

///Maximum time to optimize (in ms). Applied to linear relaxatio and MIP independently.
const LINOPT_MAX_TM: i32 = 1000;

struct Problem {
   prob: *mut naevc::glp_prob,
}
unsafe impl Sync for Problem {}
unsafe impl Send for Problem {}

impl Drop for Problem {
   fn drop(&mut self) {
      unsafe {
         naevc::glp_delete_prob(self.prob);
      }
   }
}
impl Problem {
   fn new(name: String, ncols: u32, nrows: u32) -> Result<Self> {
      let prob = unsafe {
         let p = naevc::glp_create_prob();
         if p.is_null() {
            anyhow::bail!("Failed to create optimization problem");
         }
         naevc::glp_set_prob_name(p, CString::new(name)?.as_ptr());
         naevc::glp_add_cols(p, ncols as i32);
         naevc::glp_add_rows(p, nrows as i32);
         p
      };
      Ok(Self { prob })
   }

   fn maximize(&self) {
      unsafe {
         naevc::glp_set_obj_dir(self.prob, naevc::GLP_MAX as i32);
      }
   }

   /*
   fn minimize(&self) {
      unsafe {
         naevc::glp_set_obj_dir(self.prob, naevc::GLP_MAX as i32);
      }
   }
   */

   fn num_cols(&self) -> i32 {
      unsafe { naevc::glp_get_num_cols(self.prob) }
   }

   fn num_rows(&self) -> i32 {
      unsafe { naevc::glp_get_num_rows(self.prob) }
   }
}

fn linopt_error(retval: i32) -> &'static str {
   match retval as u32 {
      0 => "No error",
      /* COMMON */
      naevc::GLP_EFAIL => "The search was prematurely terminated due to the solver failure.",
      naevc::GLP_ETMLIM => {
         "The search was prematurely terminated, because the time limit has been exceeded."
      }

      /* SIMPLEX */
      naevc::GLP_EBADB => {
         "Unable to start the search, because the initial basis specified in the problem object is invalid—the number of basic (auxiliary and structural) variables is not the same as the number of rows in the problem object."
      }
      naevc::GLP_ESING => {
         "Unable to start the search, because the basis matrix corresponding to the initial basis is singular within the working precision."
      }
      naevc::GLP_ECOND => {
         "Unable to start the search, because the basis matrix corresponding to the initial basis is ill-conditioned, i.e. its condition number is too large."
      }
      naevc::GLP_EBOUND => {
         "Unable to start the search, because some double-bounded (auxiliary or structural) variables have incorrect bounds."
      }
      naevc::GLP_EOBJLL => {
         "The search was prematurely terminated, because the objective function being maximized has reached its lower limit and continues decreasing (the dual simplex only)."
      }
      naevc::GLP_EOBJUL => {
         "The search was prematurely terminated, because the objective function being minimized has reached its upper limit and continues increasing (the dual simplex only)."
      }
      naevc::GLP_EITLIM => {
         "The search was prematurely terminated, because the simplex iteration limit has been exceeded."
      }

      /* INTOPT */
      naevc::GLP_EROOT => {
         "Unable to start the search, because optimal basis for initial LP relaxation is not provided. (This code may appear only if the presolver is disabled.)"
      }
      naevc::GLP_ENOPFS => {
         "Unable to start the search, because LP relaxation of the MIP problem instance has no primal feasible solution. (This code may appear only if the presolver is enabled.)"
      }
      naevc::GLP_ENODFS => {
         "Unable to start the search, because LP relaxation of the MIP problem instance has no dual feasible solution. In other word, this code means that if the LP relaxation has at least one primal feasible solution, its optimal solution is unbounded, so if the MIP problem has at least one integer feasible solution, its (integer) optimal solution is also unbounded. (This code may appear only if the presolver is enabled.)"
      }
      naevc::GLP_EMIPGAP => {
         "The search was prematurely terminated, because the relative mip gap tolerance has been reached."
      }
      naevc::GLP_ESTOP => {
         "The search was prematurely terminated by application. (This code may appear only if the advanced solver interface is used.)"
      }

      _ => "Unknown error.",
   }
}

fn linopt_status(retval: i32) -> &'static str {
   match retval as u32 {
      naevc::GLP_OPT => "solution is optimal",
      naevc::GLP_FEAS => "solution is feasible",
      naevc::GLP_INFEAS => "solution is infeasible",
      naevc::GLP_NOFEAS => "problem has no feasible solution",
      naevc::GLP_UNBND => "problem has unbounded solution",
      naevc::GLP_UNDEF => "solution is undefined",
      _ => "unknown GLPK status",
   }
}

impl UserData for Problem {
   fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
      /*@
       * @brief Opens a new linopt.
       *
       *    @luatparam[opt=nil] string name Name of the optimization program.
       *    @luatparam number cols Number of columns in the optimization program.
       *    @luatparam number rows Number of rows in the optimization program.
       *    @luatparam[opt=false] boolean maximize Whether to maximize (rather than
       * minimize) the function.
       *    @luatreturn Optim New linopt object.
       * @luafunc new
       */
      methods.add_function(
         "new",
         |_,
          (name, cols, rows, max): (Option<String>, u32, u32, Option<bool>)|
          -> mlua::Result<Self> {
            let max = max.unwrap_or(false);

            let prob = Problem::new(name.unwrap_or("UNKNOWN".to_string()), cols, rows)?;
            if max {
               prob.maximize();
            }
            Ok(prob)
         },
      );

      /*@
       * @brief Gets the size of the linear program.
       *
       *    @luatparam LinOpt lp Linear program to modify.
       *    @luatreturn number Number of columns in the linear program.
       *    @luatreturn number Number of rows in the linear program.
       * @luafunc size
       */
      methods.add_method("size", |_, this, ()| -> mlua::Result<(i32, i32)> {
         Ok((this.num_cols(), this.num_rows()))
      });

      /*@
       * @brief Adds columns to the linear program.
       *
       *    @luatparam LinOpt lp Linear program to modify.
       *    @luatparam number cols Number of columns to add.
       * @luafunc add_cols
       */
      methods.add_method("add_cols", |_, this, cols: i32| -> mlua::Result<()> {
         unsafe {
            naevc::glp_add_cols(this.prob, cols);
         }
         Ok(())
      });

      /*@
       * @brief Adds rows to the linear program.
       *
       *    @luatparam LinOpt lp Linear program to modify.
       *    @luatparam number rows Number of rows to add.
       * @luafunc add_rows
       */
      methods.add_method("add_rows", |_, this, rows: i32| -> mlua::Result<()> {
         unsafe {
            naevc::glp_add_rows(this.prob, rows);
         }
         Ok(())
      });

      /*@
       * @brief Adds an optimization column.
       *
       *    @luatparam LinOpt lp Linear program to modify.
       *    @luatparam number index Index of the column to set.
       *    @luatparam string name Name of the column being added.
       *    @luatparam number coefficient Coefficient of the objective function being
       * added.
       *    @luatparam[opt="real"] string kind Kind of the column being added. Can be
       * either `"real"`, `"integer"`, or `"binary"`.
       *    @luatparam[opt=nil] number lb Lower bound of the column.
       *    @luatparam[opt=nil] number ub Upper bound of the column.
       * @luafunc set_col
       */
      methods.add_method(
         "set_col",
         |_,
          this,
          (idx, name, coef, skind, lb, ub): (
            i32,
            BorrowedStr,
            f64,
            BorrowedStr,
            Option<f64>,
            Option<f64>,
         )|
          -> mlua::Result<()> {
            let typ = if let Some(lb) = lb
               && let Some(ub) = ub
            {
               if (lb - ub).abs() < naevc::DOUBLE_TOL {
                  naevc::GLP_FX
               } else {
                  naevc::GLP_DB
               }
            } else if lb.is_some() {
               naevc::GLP_LO
            } else if ub.is_some() {
               naevc::GLP_UP
            } else {
               naevc::GLP_FR
            };

            let kind = match skind.as_ref() {
               "real" => naevc::GLP_CV,
               "integer" => naevc::GLP_IV,
               "binary" => naevc::GLP_BV,
               _ => {
                  return Err(mlua::Error::RuntimeError(format!(
                     "unknown column kind '{}'",
                     skind
                  )));
               }
            };

            unsafe {
               naevc::glp_set_col_name(
                  this.prob,
                  idx,
                  CString::new(name.as_bytes()).unwrap().as_ptr(),
               );
               naevc::glp_set_obj_coef(this.prob, idx, coef);
               naevc::glp_set_col_bnds(
                  this.prob,
                  idx,
                  typ as i32,
                  lb.unwrap_or(0.0),
                  ub.unwrap_or(0.0),
               );
               naevc::glp_set_col_kind(this.prob, idx, kind as i32);
            }
            Ok(())
         },
      );

      /*@
       * @brief Adds an optimization row.
       *
       *    @luatparam LinOpt lp Linear program to modify.
       *    @luatparam number index Index of the row to set.
       *    @luatparam string name Name of the row being added.
       *    @luatparam[opt=nil] number lb Lower bound of the row.
       *    @luatparam[opt=nil] number ub Upper bound of the row.
       * @luafunc set_row
       */
      methods.add_method(
         "set_row",
         |_,
          this,
          (idx, name, lb, ub): (i32, BorrowedStr, Option<f64>, Option<f64>)|
          -> mlua::Result<()> {
            let typ = if let Some(lb) = lb
               && let Some(ub) = ub
            {
               if (lb - ub).abs() < naevc::DOUBLE_TOL {
                  naevc::GLP_FX
               } else {
                  naevc::GLP_DB
               }
            } else if lb.is_some() {
               naevc::GLP_LO
            } else if ub.is_some() {
               naevc::GLP_UP
            } else {
               naevc::GLP_FR
            };

            unsafe {
               naevc::glp_set_row_name(
                  this.prob,
                  idx,
                  CString::new(name.as_bytes()).unwrap().as_ptr(),
               );
               naevc::glp_set_row_bnds(
                  this.prob,
                  idx,
                  typ as i32,
                  lb.unwrap_or(0.0),
                  ub.unwrap_or(0.0),
               );
            }
            Ok(())
         },
      );

      /*@
       * @brief Loads the entire matrix for the linear program.
       *
       *    @luatparam LinOpt lp Linear program to modify.
       *    @luatparam number row_indices Indices of the rows.
       *    @luatparam number col_indices Indices of the columns.
       *    @luatparam number coefficients Values of the coefficients.
       * @luafunc load_matrix
       */
      methods.add_method(
         "load_matrix",
         |_, this, (row, col, coef): (mlua::Table, mlua::Table, mlua::Table)| -> mlua::Result<()> {
            let n = coef.raw_len();

            #[cfg(debug_assertions)]
            if row.raw_len() != n || col.raw_len() != n {
               return Err(mlua::Error::RuntimeError(
                  "Table lengths don't match!".to_string(),
               ));
            }

            // glp_load_matrix accesses from 1..=n, so we can't just use Vec<i32> or whatever that would
            // be much cleaner...
            let mut ia = vec![0i32; n + 1];
            let mut ja = vec![0i32; n + 1];
            let mut ar = vec![0f64; n + 1];
            for i in 1..=n {
               ia[i] = row.raw_get(i)?;
               ja[i] = col.raw_get(i)?;
               ar[i] = coef.raw_get(i)?;
            }

            unsafe {
               naevc::glp_load_matrix(this.prob, n as i32, ia.as_ptr(), ja.as_ptr(), ar.as_ptr());
            }

            Ok(())
         },
      );

      /*@
       * @brief Solves the linear optimization problem.
       *
       *    @luatparam LinOpt lp Linear program to modify.
       *    @luatreturn number The value of the primal function.
       *    @luatreturn table Table of column values.
       * @luafunc solve
       */
      methods.add_method(
         "solve",
         |_, this, _params: Option<Table>| -> mlua::Result<(f64, Vec<f64>, Vec<f64>)> {
            use std::mem::MaybeUninit;

            let mut parm_smcp = unsafe {
               let mut parm_smcp = MaybeUninit::<naevc::glp_smcp>::uninit();
               naevc::glp_init_smcp(parm_smcp.as_mut_ptr());
               parm_smcp.assume_init()
            };
            parm_smcp.msg_lev = naevc::GLP_MSG_ERR as i32;
            parm_smcp.tm_lim = LINOPT_MAX_TM;
            let mut parm_iocp = unsafe {
               let mut parm_iocp = MaybeUninit::<naevc::glp_iocp>::uninit();
               naevc::glp_init_iocp(parm_iocp.as_mut_ptr());
               parm_iocp.assume_init()
            };
            parm_iocp.msg_lev = naevc::GLP_MSG_ERR as i32;
            parm_iocp.tm_lim = LINOPT_MAX_TM;

            // MIP problem
            let ismip = unsafe { naevc::glp_get_num_int(this.prob) > 0 };

            // TODO handle parameters
            if !ismip || parm_iocp.presolve == 0 {
               let ret = unsafe { naevc::glp_simplex(this.prob, &parm_smcp) } as i32;
               if ret != 0 && ret != naevc::GLP_ETMLIM as i32 {
                  return Err(mlua::Error::RuntimeError(linopt_error(ret).to_string()));
               }
               // Check for optimality of the continuous problem
               let ret = unsafe { naevc::glp_get_status(this.prob) };
               if ret != naevc::GLP_OPT as i32 && ret != naevc::GLP_FEAS as i32 {
                  return Err(mlua::Error::RuntimeError(linopt_status(ret).to_string()));
               }
            }
            if ismip {
               let ret = unsafe { naevc::glp_intopt(this.prob, &parm_iocp) };
               if ret != 0 && ret != naevc::GLP_ETMLIM as i32 {
                  return Err(mlua::Error::RuntimeError(linopt_error(ret).to_string()));
               }
               let ret = unsafe { naevc::glp_mip_status(this.prob) };
               if ret != naevc::GLP_OPT as i32 && ret != naevc::GLP_FEAS as i32 {
                  return Err(mlua::Error::RuntimeError(linopt_status(ret).to_string()));
               }
            }

            let z = unsafe { naevc::glp_get_obj_val(this.prob) };
            let cols = (1..=this.num_cols())
               .map(|i| {
                  if ismip {
                     unsafe { naevc::glp_mip_col_val(this.prob, i) }
                  } else {
                     unsafe { naevc::glp_get_col_prim(this.prob, i) }
                  }
               })
               .collect();
            let rows = (1..=this.num_rows())
               .map(|i| {
                  if ismip {
                     unsafe { naevc::glp_mip_row_val(this.prob, i) }
                  } else {
                     unsafe { naevc::glp_get_row_prim(this.prob, i) }
                  }
               })
               .collect();

            Ok((z, cols, rows))
         },
      );
   }
}

pub fn open_linopt(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
   Ok(lua.create_proxy::<Problem>()?)
}
