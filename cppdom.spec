# Spec file for cppdom.
%define name    cppdom
%define version 0.7.7
%define release 1

Name: %{name}
Summary: A C++ based XML loader and writer with an internal DOM representation.
Version: %{version}
Release: %{release}
Source: %{name}-%{version}.tar.gz
URL: http://www.sf.net/projects/xml-cppdom/
Group: Development/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
License: LGPL
BuildPrereq: scons >= 0.96.1
Vendor: xml-cppdom Project
Provides: cppdom = %{version}-%{release}
Obsoletes: cppdom < %{version}-%{release}

%description
CppDOM is a C++ based XML loader and writer with an internal DOM
representation. It is lightweight and high-performance. The goal of the
project is to provide a lightweight C++ interface for XML programming that
is similar to the API and functionality of JDOM.

%package devel
Summary: Header files and libraries needed for CppDOM development
Group: Development/C++
Requires: cppdom = %{version}-%{release}
Provides: cppdom-devel = %{version}-%{release}
Conflicts: cppdom-devel < %{version}-%{release}

%description devel
The header files and libraries needed for developing programs using CppDOM.

%prep
rm -rf %{buildroot}
%setup -q

%build
lib_subdir=`echo %{_libdir} | sed -e "s|%{_prefix}/\(.*\)|\1|"`
CXXFLAGS="$RPM_OPT_FLAGS"
LINKFLAGS="$RPM_OPT_FLAGS"
export CXXFLAGS
export LINKFLAGS
scons prefix=%{buildroot}/usr libdir=$lib_subdir optimize=yes build_test=no

%install
[ -z %{buildroot} ] || rm -rf %{buildroot}

lib_subdir=`echo %{_libdir} | sed -e "s|%{_prefix}/\(.*\)|\1|"`
CXXFLAGS="$RPM_OPT_FLAGS"
LINKFLAGS="$RPM_OPT_FLAGS" 
export CXXFLAGS
export LINKFLAGS
scons prefix=%{buildroot}%{_prefix} libdir=$lib_subdir build_test=no install
# Remove all stupid scons temp files
find %{buildroot}%{_prefix} -name .sconsign -exec rm {} \;

%clean
[ -z %{buildroot} ] || rm -rf %{buildroot}

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-, root, root)
%{_libdir}/*.so
%doc README AUTHORS ChangeLog COPYING

%files devel
%defattr(-, root, root)
%{_bindir}/cppdom-config
%{_includedir}/cppdom/*.h
%{_includedir}/cppdom/ext/*.h
%{_libdir}/*.a

%changelog
* Wed Apr 12 2006 Patrick Hartling
- Updated to handle multi-architecture installations.

* Mon Jun 06 2005 Aron Bierbaum
- Total rewrite

* Thu Mar 13 2003 Allen Bierbaum
- First version
