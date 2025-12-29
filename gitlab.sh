glab auth login


# Create a merge request
glab mr create --fill --title "My feature" --description "Details"

# List merge requests
glab mr list

# View a specific merge request
glab mr view 123

# Merge a merge request
glab mr merge 123

# Approve an MR
glab mr approve 123

# Checkout an MR locally
glab mr checkout 123





# View issues
glab issue list

# Create an issue
glab issue create -t "Bug title" -d "Bug details"

# View pipelines
glab pipeline list

# View CI/CD jobs
glab pipeline ci view
