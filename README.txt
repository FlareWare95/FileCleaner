FileCleaner is a tool that will help manage disk space on a user's device by encouraging them to review unused files before deciding on whether they need to upload those to a company's sharepoint, or deleted instead. The FileCleaner works in two parts: 
	
	FileFinder: Run when specified, attempts to move any file or folder 	inside of the current user's Downloads folder older than a month (or 	whenever specified) to a new folder on the user's desktop, where they can 	review these files. Additionally, FileFinder will output a log file that	gives the user (and the administrators) a human-readable summary of all 	of the files that the program moved.

	FolderClear: Run a set time after the FileFinder, FolderClear will simply	move the files left in the folder to the recycle bin OR wherever files 		flagged for deletion should go. 

The times that these tools will run should be set using scheduled tasks, which can be configured via a Powershell script that then can be run through enterprise tools like Microsoft Intune. 

While not yet implemented, there will be an overhaul regarding security and flexibility. For instance:

	- Easy settings configuration through .txt file or json instead of 	  	  cmdline options like it currently has
	- SHA-256 encryption to ensure no tampering of said config file has 		  happened
	- Any other security is recommended to me

This project is on GitHub: github.com/FlareWare95/FileCleaner.