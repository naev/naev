#!/bin/bash

set -e
openssl aes-256-cbc -K $encrypted_18d5d806b7a8_key -iv $encrypted_18d5d806b7a8_iv -in .travis/deploy_key.pem.enc -out .travis/deploy_key.pem -d
eval "$(ssh-agent -s)"
chmod 600 .travis/deploy_key.pem
ssh-add .travis/deploy_key.pem
rsync -e "ssh -o 'StrictHostKeyChecking no'" -rv --delete docs/html/ travis@iandouglasscott.com:/srv/naevdoc
