#
# Spec file for cppdom
#
Summary: A C++ xml API for use with STL
Name: cppdom
Version: 0.4.0
Release: 1
Copyright: LGLP
Group: Libraries
#Source0: none
URL: http://www.sf.net/projects/xml-cppdom
#Epoch: 1
Vendor: xml-cppdom project
Packager: Allen Bierbaum

BuildRoot: /var/tmp/allenb/Source/cppdom-0.4.0


%description
This is the xml-cppdom library for fast access to xml trees
in a very STL natural way.

%files
%defattr(-, root, root)
/include
/bin
/lib

%changelog
* Thu Mar 13 2003 Allen Bierbaum
- First version
