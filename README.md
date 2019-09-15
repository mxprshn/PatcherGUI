# PatcherGUI

*Graphical user interface for PostgreSQL database patch Builder & Installer [https://github.com/bazhulinnv/Patcher]*

## Getting started

**Work with GUI requires a pgpass.conf file in a user's home directory.** On Microsoft Windows the file is named %APPDATA%\postgresql\pgpass.conf
(where %APPDATA% refers to the Application Data subdirectory in the user's profile). [https://www.postgresql.org/docs/9.3/libpq-pgpass.html]
This file should contain lines of the following format: `hostname:port:database:username:password` for all databases used with the application.

After launching the application you should establish connection to a database. This can be done by clicking database button in the top-left
window corner and entering the connection parameters.

## Building a patch

After connection to a database you can start building a patch. The "Build" tab of the main window has a convenient interface for patch list 
creation. You should specify type, schema and name of the object which is being added with pop-up menus. After the "Add" button is clicked, 
the object will be added to the patch list in case of its existence in current database. Notice that function signature input should be in
specific format in order to be successfully found. SQL script files can be added to the list as well as concrete database objects to be
executed during the patch installation. To do this, choose "script" as the object type and enter the path to `.sql` files in name edit, or leave
it empty to choose files from explorer. Objects in the patch list can be rearranged with "Up" and "Down" buttons or using drag-and-drop to be
installed in particular order later.

When the list is created, you should specify the directory where the patch files will be generated, and click "Build" button. As an option, the path
to `Templates.ini` configuration file for the Builder module can be set in settings window (Main Menu -> Settings...).

## Installing a patch

To install a built patch, switch to the "Install" tab of the main window. There you should enter the patch directory path (leave the edit empty to choose it in explorer).
After the "Open" button is clicked and the directory is opened, you can see the list of objects which are referenced by the objects in patch ("Dependencies", on the left) and the list of
patch objects themselves ("Patch Objects", on the right).

To enable installation, you should firstly launch dependency check process by clicking the "Check" button. When it is finished, you can see how
the warning icons in dependency list are replaced with checks (for found dependencies) or crosses (otherwise). As it is significant to pay the user's
attention to the unsatisfied dependencies, installation can be launched only after he marks all the objects in the dependency list manually (satisfied
dependencies are marked automatically). When it is done, the "Install" will be enabled.
