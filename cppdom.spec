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
Group: Development/Libraries
#Source0: none
URL: http://www.sf.net/projects/xml-cppdom
#Epoch: 1
Vendor: xml-cppdom Project
Packager: Allen Bierbaum

BuildRoot: /var/tmp/cppdom-root

%description
CppDom is a C++ based XML loader and writer with an internal DOM representation.
It is very lightweight and high-performance.  The goal of the project is to
provide a lightweight C++ interface for XML programming that is similar to
the API and functionality of JDOM.

%files
%defattr(-, root, root)
/include
/lib
/test

%changelog
* Thu Mar 13 2003 Allen Bierbaum
- First version
