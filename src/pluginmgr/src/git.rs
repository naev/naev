use crate::install::Progress;
use anyhow::Error;
use iced::task::{Straw, sipper};
use std::path::Path;

/*
pub fn clone<P: AsRef<Path>, U: reqwest::IntoUrl>( path: P, url: U ) -> impl Straw<(), Progress, Error> {
    sipper( async move |sender| {
        // Prepare callbacks.
        let mut callbacks = git2::RemoteCallbacks::new();
        callbacks.transfer_progress( move |prog: git2::Progress| {
            let p = (prog.received_objects() as f32) / (prog.total_objects() as f32);
            //let handle = iced::futures::backend::tokio::Handle::current();
            sender.send( Progress {
                message: None,
                value: p,
            } ).await;
            true
        } );

        // Prepare fetch options.
        let mut fo = git2::FetchOptions::new();
        fo.remote_callbacks(callbacks);

        // Prepare builder.
        git2::build::RepoBuilder::new()
            .fetch_options(fo)
            .clone( url.as_str(), path.as_ref() )?;

        Ok(())
    } )
}
*/
