use crate::install::Progress;
use anyhow::{Context, Error};
use iced::task::{Straw, sipper};
use std::path::Path;

pub fn clone<P: AsRef<Path>, U: reqwest::IntoUrl>(
    path: P,
    url: U,
) -> impl Straw<git2::Repository, Progress, Error> {
    sipper(async move |mut sender| {
        // Have to wrap the async sender.send to make it a FnMut
        let (send, recv) = tokio::sync::mpsc::unbounded_channel();
        let on_progress = move |item| {
            send.send(item).unwrap();
        };
        tokio::spawn(async move {
            let mut recv = recv;
            while let Some(item) = recv.recv().await {
                sender.send(item).await;
            }
        });

        // Prepare callbacks.
        let mut callbacks = git2::RemoteCallbacks::new();
        callbacks.transfer_progress(move |prog: git2::Progress| {
            let p = (prog.received_objects() as f32) / (prog.total_objects() as f32);
            on_progress(Progress {
                message: None,
                value: p,
            });
            true
        });

        // Prepare fetch options.
        let mut fo = git2::FetchOptions::new();
        fo.remote_callbacks(callbacks);

        // Clone
        Ok(git2::build::RepoBuilder::new()
            .fetch_options(fo)
            .clone(url.as_str(), path.as_ref())?)
    })
}

pub fn pull<P: AsRef<Path>>(path: P) -> impl Straw<git2::Repository, Progress, Error> {
    sipper(async move |mut sender| {
        // Have to wrap the async sender.send to make it a FnMut
        let (send, recv) = tokio::sync::mpsc::unbounded_channel();
        let on_progress = move |item| {
            send.send(item).unwrap();
        };
        tokio::spawn(async move {
            let mut recv = recv;
            while let Some(item) = recv.recv().await {
                sender.send(item).await;
            }
        });

        // Prepare callbacks.
        let mut callbacks = git2::RemoteCallbacks::new();
        callbacks.transfer_progress(move |prog: git2::Progress| {
            let p = (prog.received_objects() as f32) / (prog.total_objects() as f32);
            on_progress(Progress {
                message: None,
                value: p,
            });
            true
        });

        // Open the repo
        let repo = git2::Repository::open(path.as_ref())?;
        {
            // Prepare fetch options.
            let mut fo = git2::FetchOptions::new();
            fo.remote_callbacks(callbacks);

            // Connect to the remote repository.
            let remote_name = "origin";
            let mut remote = repo.find_remote(remote_name)?;
            remote.connect(git2::Direction::Fetch)?;

            // Get the default branch name of the remote repository.
            let default_branch = remote.default_branch()?;
            let default_branch_name = default_branch
                .as_str()
                .context("Failed to get default branch name")?;

            // Execute the fetch operation.
            remote.fetch(&[default_branch_name], Some(&mut fo), None)?;

            // Find the FETCH_HEAD reference.
            let fetch_head = repo.find_reference("FETCH_HEAD")?;

            // Convert the FETCH_HEAD reference to an annotated commit.
            let fetch_commit = repo.reference_to_annotated_commit(&fetch_head)?;

            // Get the merge analysis result.
            let analysis = repo.merge_analysis(&[&fetch_commit])?;

            // If a fast-forward merge is possible, then perform the merge operation.
            if analysis.0.is_fast_forward() {
                let mut reference = repo.find_reference(default_branch_name)?;
                reference.set_target(fetch_commit.id(), "Fast forward")?;
                repo.set_head(default_branch_name)?;
                repo.checkout_head(Some(git2::build::CheckoutBuilder::default().force()))?
            }
        }
        Ok(repo)
    })
}
