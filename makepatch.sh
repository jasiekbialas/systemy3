
ORIG_FOLDER="$(pwd)"
REPO="$(git rev-parse --show-toplevel)" # get the repo root folder
PATCH_FILE="jb385150.patch" # set the output file
ORIGINAL_BRANCH="master"
SOLUTION_BRANCH="zad3"

cd "$REPO/minix_source" # cd into the folder that contains the usr folder
git diff --relative "$ORIGINAL_BRANCH" "$SOLUTION_BRANCH" -- usr > $PATCH_FILE
mv "$PATCH_FILE" "$ORIG_FOLDER/$PATCH_FILE"
