import os, apport.packaging
from apport.hookutils import *

def add_info(report):
	# the issue is not in the gnome-control-center code so reassign
	if report.has_key("Stacktrace") and "/usr/lib/control-center-1" in report["Stacktrace"]:
		for words in report["Stacktrace"].split():
			if words.startswith("/usr/lib/control-center-1"):
			    if apport.packaging.get_file_package(words) != 'gnome-control-center':
    				report.add_package_info(apport.packaging.get_file_package(words))
    				return

	# collect informations on the /usr/lib/control-center-1 components 
	plugin_packages = set()
	for dirpath, dirnames, filenames in os.walk("/usr/lib/control-center-1"):
		for filename in filenames:
			path = os.path.join(dirpath, filename)
			package = apport.packaging.get_file_package(path)
			if package == 'gnome-control-center':
				continue

			plugin_packages.add(package)

	report["usr_lib_gnome-control-center"] = package_versions(*sorted(plugin_packages))

