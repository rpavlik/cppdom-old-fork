#
# Spec file for cppdom
#rpmbuild -bb -v --define='_topdir /var/tmp/cppdom' --define='_rpmdir /var/tmp/cppdom' --buildroot=/..../cppdom/build.linux/dist/cppdom-0.5.3 cppdom.spec
# Run as: 
#
Summary: A C++ xml API for use with STL
# Name: cppdom
# Version: 0.5.2
# Release: 1
Name: _SCONS_PACKAGE_NAME_
Version: _SCONS_PACKAGE_VERSION_
Release: _SCONS_PACKAGE_RELEASE_
Copyright: LGLP
Group: Libraries
#Source0: none
URL: http://www.sf.net/projects/xml-cppdom
#Epoch: 1
Vendor: xml-cppdom project
Packager: Allen Bierbaum

BuildRoot: /var/tmp/cppdom-root

%description
This is the xml-cppdom library for fast access to xml trees
in a very STL natural way.

%files
%defattr(-, root, root)
/include
/lib
/test

%changelog
* Thu Mar 13 2003 Allen Bierbaum
- First version
