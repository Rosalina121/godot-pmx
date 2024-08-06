# MMD asset importer for Godot

## What is it?

Fork of a Godot 4* module that imports MMD assets. It currently has
incomplete support for PMX models, but we intend to add at least VMD
motions as well.

*Tested with 4.3.

## How do I use it?

We develop against Godot's `master` branch, so first check that out
with Git and make sure you can compile it. Then clone this repository
as `modules/pmx` under the Godot root and build again. At that point
you should be able to import PMX models inside your Godot project
folders. They will show up as packed scenes.

## Disclaimer
This fork for now is just a fix to make the archived module work with 4.3.
For now I do not plan to implement any features mentioned in the old repo issues, as I also have limited knowledge regarding MMD topics.
TBH I forked this so we can use it in our project.
If you'd like to help, feel free to do so :)
