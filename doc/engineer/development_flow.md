Development flow
================
General guidelines for daily workflow

Git setup
---------
It's recommended to read the [local environment setup doc](local_setup.md) first.

[the main article on git setup for Gardens is also on the intranet](https://i.acquia.com/wiki/managing-gardens-codebase-git) 

Typical development flow
------------------------
**TL;DR:**

 - make sure you have been through the [local setup doucment](local_setup.md) to ensure that your conflict resolutions are properly shared automatically
 - Name branches after JIRA stories
 - Commit regularly, but not every single line change
 - Merge to integration  as often as you push to origin, resolving conflicts and make sure the resolution is pushed 
 - avoid force pushes unless you're **really sure** you're not going to break the branch


Every piece of work needs an issue of some sort in JIRA.  Your git branch will be named after it and this naming convention is used to determine what gets released.  If your branch is misnamed, it probably won't be released until named correctly.

If working on a major "story" in JIRA, then the branch is named after the top level story, not each of the subtasks.  If you have a bug to fix which has no JIRA issue, create one first, and name your branch after it.  If the task will take you more than a couple of minutes to complete, make sure to include enough detail in the ticket so that someone else could possibly take it over if needed.  In either case, your branch will be based on master (occasionally it will needed to be based on a release candidate branch, if it depends on something in the release candidate - typically urgent fixes coming up late in the sprint):

    git branch -b <name> origin/master

Assuming you're working on your own locally, you can develop at will on this branch.  It is recommended that commits are made in discrete, logical chunks.  More frequent commits are preferred over one large commit at the end, but ideally there is a balance to be found between committing every small detail and one huge commit at the end of an issue.

If another team member is working on the same branch, you'll probably need to pull their changes from time to time, and you won't be able to push if there are incoming changes.  Your [git setup](local_setup.md) should ensure that incoming changes are rebased (to keep history clean)
, but you can include the rebase parameter for good measure:

    git pull --rebase


When it comes time to push locally committed changes back up to github, you have the option of doing a dry run without actually pushing.  This gives you a chance to review exactly the changes you're pushing (which may be more than one set of changes)

    git push --dry-run origin <my-branch>
    
Git will ouput the revision range that you can plug back directly into diff to see exactly what's changing (a dry run is a good sanity check, but not required):

    To git@github.com:acquia/gardens.git
      9b5567b..c87f5cb  DG-6676 -> DG-6676

    git diff 9b5567b..c87f5cb
    
You can have a quick check through the diff, after which push again without the --dry-run parameter.

Now is a good point to merge into integration.  The reason for doing this is so that the developer introducing merge conflicts can merge them him/herself, pushing the resolution to origin also so that it is available when this same conflict comes up at release time.  It is never expected that the integration branch is actually run on any environment - its sole purpose is creating merge conflict resolutions which can be reused by the release engineer creating the release candidate branch:

    git checkout integration
    git pull --rebase
    git merge --no-ff <my branch>
    
At the point, resolve any conflicts, commit and push integration.

JIRA
----

This is a summary of the more complete [definition of done document](https://docs.google.com/a/acquia.com/document/d/1BP6WZi61IT-jXqdmuTQ6Agm1NhbgS9Q9qBfF8Mrjfk8/edit)

To be continued ...

