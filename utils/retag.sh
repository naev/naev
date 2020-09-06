#!/bin/bash
# Retags any tags that apply to the wildcard in semver

# Pass in -w <wildcard>

set -e

while getopts d:w: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    w)
        WILD=${OPTARG}
        ;;
    esac
done

replace_tag () {
    for wildcard in $(echo $1 | tr [a-z] [A-Z]) $1
    do
        for tag in $(git tag -l "$wildcard*")
        do
        if [[ -n $(echo $tag | grep $wildcard ) ]]; then
            echo $tag
            if [[ -n $(echo $tag | grep "-beta" ) ]]; then
                betanum=${tag#*-beta} 
                newtag="v"${tag#*-}
                newtag=${newtag%[0-9]}.$betanum
            else
                newtag="v"${tag#*-}
            fi
            echo $newtag
            #commit=$(git rev-parse $tag)
            git tag $newtag $tag^{}
            git tag -d $tag
            #echo git tag -a $newtag -m "replaced $tag with $newtag" $commit
            git push origin :refs/tags/$tag
        fi
        done
    done
    git push origin --tags
}

replace_tag "$WILD"
