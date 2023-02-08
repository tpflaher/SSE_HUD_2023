Doxygen is an open source tool for generating documentation directly from code!
You can install it here: https://www.doxygen.nl/download.html

The basic idea is that you have a configuration file called "Doxyfile" in your code
and the settings there are then used by the doxygen utility to generate HTML(like a website) documentation. 

I used this tutorial to set the correct settings for our configuration file
https://www.doxygen.nl/download.html

This documentation can then either be hosted on a website, or read locally.
All modern browsers are able to read html files, so if you put the full file path to the docs/html/index.html
file into your browsers search bar than it should show you the documentation page.

Whenever a change is made to the code, updating the documentation is as simple as navigating to the
correct folder in the command line, and then running the
'doxygen' command!