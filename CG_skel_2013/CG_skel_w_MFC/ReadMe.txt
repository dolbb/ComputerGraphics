
To add input dialog boxes support to your project you need to:


1. Copy the following files to the 'CG_skel_w_MFC' directory:

	- InputDialog.cpp

	- InputDialog.h

	- Resource.h (replace the old one)

	- CG_skel_w_MFC.rc (replace the old one)


2. From within the solution, add to the CG_skel_w_MFC project - the header 'InputDialog.h' and
   the source 'InputDialog.cpp' (as existing items).


3. Clean and then build the project.

4. How to use?
   There are 3 dialog classes for you to choose frome:

	- CCmdDialog (reads a string from the user)

	- CXyzDialog (reads x, y, z from the user)

	- CCmdXyzDialog (reads a string and x, y, z from the user)

   When you wish to open a dialog with the user, you need to instansiate a dialog class, invoke the DoModal
   method and when the user presses OK, retrieve the input with the 'Get' methods.

   For example:

   	CCmdXyzDialog dlg;
	if (dlg.DoModal() == IDOK) {
		string command = dlg.GetCmd();
		vec3 v = dlg.GetXYZ();

		...			
	}
