#!/usr/bin/env python3

import argparse
import os
import sys
import logging
import json
import mimetypes
import time
import urllib.parse
import urllib.request
import urllib.error

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

def parse_args():
   parser = argparse.ArgumentParser(description="Upload a release and assets to a Forgejo (Gitea-compatible) instance.")
   parser.add_argument("--tag", required=True, help="Release tag to create/update.")
   parser.add_argument("--asset-dir", required=True, help="Directory containing assets to upload.")
   parser.add_argument("--title", help="Release title (defaults to tag).")
   parser.add_argument("--notes", help="Release notes (inline text).")
   parser.add_argument("--notes-file", help="Release notes file (overrides --notes).")
   parser.add_argument("--owner", default=os.environ.get("FORGEJO_OWNER", "naev"), help="Repository owner.")
   parser.add_argument("--repo", default=os.environ.get("FORGEJO_REPO", "naev"), help="Repository name.")
   parser.add_argument("--api-url", default=os.environ.get("FORGEJO_API_URL", "https://codeberg.org/api/v1"), help="API base URL.")
   parser.add_argument("--prerelease", action="store_true", help="Mark release as prerelease.")
   parser.add_argument("--overwrite", action="store_true", help="Delete existing release with same tag first.")
   parser.add_argument("--hide-archive-link", action="store_true", help="Hide auto-generated source archives on the release.")
   return parser.parse_args()

def get_token():
   token = os.environ.get("FORGEJO_TOKEN") or os.environ.get("CODEBERG_TOKEN") or os.environ.get("GITHUB_TOKEN")
   if not token:
      logging.error("FORGEJO_TOKEN (or CODEBERG_TOKEN / GITHUB_TOKEN) must be set")
      sys.exit(1)
   return token

def api_request(url, method="GET", headers=None, data=None):
   if headers is None:
      headers = {}
   
   if data is not None:
      if isinstance(data, (dict, list)):
         data = json.dumps(data).encode("utf-8")
         headers["Content-Type"] = "application/json"
      elif isinstance(data, str):
         data = data.encode("utf-8")
   
   req = urllib.request.Request(url, data=data, headers=headers, method=method)
   
   try:
      with urllib.request.urlopen(req) as response:
         if response.status == 204:
            return None
         return json.loads(response.read().decode("utf-8"))
   except urllib.error.HTTPError as e:
      # Return the error response body if possible for debugging
      try:
         err_body = e.read().decode("utf-8")
         logging.error(f"HTTP Error {e.code}: {e.reason} - {err_body}")
      except:
         logging.error(f"HTTP Error {e.code}: {e.reason}")
      raise

def retry_api_request(url, method="GET", headers=None, data=None, attempts=5):
   delay = 1
   for i in range(attempts):
      try:
         return api_request(url, method, headers, data)
      except urllib.error.HTTPError as e:
         if i == attempts - 1:
            raise
         logging.warning(f"Attempt {i+1}/{attempts} failed, retrying in {delay}s...")
         time.sleep(delay)
         delay *= 2
   return None

def main():
   args = parse_args()
   token = get_token()
   
   if not os.path.isdir(args.asset_dir):
      logging.error(f"Asset dir '{args.asset_dir}' not found")
      sys.exit(1)

   headers = {
      "Authorization": f"token {token}",
   }

   base_url = args.api_url.rstrip("/")
   repo_url = f"{base_url}/repos/{args.owner}/{args.repo}"

   # Prepare body
   body_content = args.notes if args.notes else ""
   if args.notes_file:
      try:
         with open(args.notes_file, "r") as f:
            body_content = f.read()
      except Exception as e:
         logging.error(f"Failed to read notes file: {e}")
         sys.exit(1)

   title = args.title if args.title else args.tag

   # Overwrite logic
   if args.overwrite:
      try:
         # Check if release exists
         try:
            release = api_request(f"{repo_url}/releases/tags/{args.tag}", headers=headers)
            if release:
               release_id = release['id']
               logging.info(f"Deleting existing release {release_id} for tag {args.tag}")
               retry_api_request(f"{repo_url}/releases/{release_id}", method="DELETE", headers=headers)
         except urllib.error.HTTPError as e:
            if e.code != 404:
               raise
      except Exception as e:
         logging.warning(f"Failed to check/delete existing release: {e}")

   # Create Release
   payload = {
      "tag_name": args.tag,
      "name": title,
      "body": body_content,
      "draft": False,
      "prerelease": args.prerelease,
      "hide_archive_links": args.hide_archive_link
   }

   try:
      logging.info(f"Creating release for tag {args.tag}")
      release = retry_api_request(f"{repo_url}/releases", method="POST", headers=headers, data=payload)
      release_id = release['id']
      logging.info(f"Created release id {release_id}")
   except Exception as e:
      logging.error(f"Failed to create release: {e}")
      sys.exit(1)

   # Upload Assets
   for filename in os.listdir(args.asset_dir):
      filepath = os.path.join(args.asset_dir, filename)
      if os.path.isfile(filepath):
         # URL encode the filename
         encoded_name = urllib.parse.quote(filename)
         logging.info(f"Uploading asset: {filename} -> {encoded_name}")
         
         upload_url = f"{repo_url}/releases/{release_id}/assets?name={encoded_name}"
         
         # Read file content
         with open(filepath, "rb") as f:
            file_data = f.read()
         
         asset_headers = headers.copy()
         asset_headers["Content-Type"] = "application/octet-stream"
         
         try:
            retry_api_request(upload_url, method="POST", headers=asset_headers, data=file_data)
         except Exception as e:
            logging.error(f"Failed to upload asset {filename}: {e}")
            sys.exit(1)

   logging.info("Release upload complete.")

if __name__ == "__main__":
   main()
