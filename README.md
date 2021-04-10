# Avnet MiniZed Tutorial

4/10/2021 D. W. Hawkins (dwh@caltech.edu)

# Introduction

This repository contains a tutorial for the Avnet MiniZed board.

The MiniZed documentation used to be hosted on the ZedBoard.org site:

http://zedboard.org/product/minized

That web page now (April 2021) redirects to an Avnet page:

https://www.avnet.com/wps/portal/us/products/avnet-boards/avnet-board-families/minized/

The "Technical Documents" and "Reference Designs" tabs at the bottom of this page contain the documentation and example designs that used to be found on the Zedboard site. The references folder of this repository contains copies of the documents and reference designs downloaded in April 2021.

# Repository Contents

Directory           | Contents
--------------------|-----------
tutorial            | MiniZed tutorial
designs             | Example designs
references          | Reference documentation

# Git Large File Storage (LFS)

This repository uses git LFS, so that binary files are stored more efficiently.

https://git-lfs.github.com/

The repository was created on github, then cloned, and then git LFS was installed via

$ git lfs install

The following binary format documents are tracked via git LFS.

$ git lfs track *.docx *.pptx *.xlsx *.pdf *.png *.jpg *.zip

The tracked file types are maintained in the .gitattributes file (which is in the repository).
