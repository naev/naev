#!/bin/bash

set -e
eval "$(ssh-agent -s)"
chmod 600 .travis/deploy_key.pem
ssh-add .travis/deploy_key.pem
rsync -e "ssh -o 'StrictHostKeyChecking no'" -rv --delete docs/html/ travis@iandouglasscott.com:/srv/naevdoc
