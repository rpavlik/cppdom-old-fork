# Spec file for cppdom.
%define name    cppdom
%define version 0.7.7
%define release 2

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
Requires: flagpoll >= 0.8.1

%description devel
The header files and libraries needed for developing programs using CppDOM.

%prep
rm -rf %{buildroot}
%setup -q

%ifarch x86_64
%define arch x64
%else
%ifarch i386 i486 i586 i686
%define arch ia32
%endif
%endif

%build
CXXFLAGS="$RPM_OPT_FLAGS"
LINKFLAGS="$RPM_OPT_FLAGS"
export CXXFLAGS
export LINKFLAGS
scons prefix=%{buildroot}%{_prefix} var_arch=%{arch} var_type=optimized build_test=no

%install
[ -z %{buildroot} ] || rm -rf %{buildroot}

lib_subdir=`echo %{_libdir} | sed -e "s|%{_prefix}/\(.*\)|\1|"`
CXXFLAGS="$RPM_OPT_FLAGS"
LINKFLAGS="$RPM_OPT_FLAGS" 
export CXXFLAGS
export LINKFLAGS
scons prefix=%{buildroot}%{_prefix} var_arch=%{arch} var_type=optimized build_test=no install
sed -i -e 's|%{buildroot}||g' %{buildroot}%{_libdir}/flagpoll/*.fpc
sed -i -e 's|%{buildroot}||g' %{buildroot}%{_bindir}/cppdom-config
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
%dir %{_includedir}/cppdom-%{version}/
%dir %{_includedir}/cppdom-%{version}/cppdom/
%{_includedir}/cppdom-%{version}/cppdom/*.h
%dir %{_includedir}/cppdom-%{version}/cppdom/ext/
%{_includedir}/cppdom-%{version}/cppdom/ext/*.h
%{_libdir}/*.a
%{_libdir}/flagpoll

%changelog
* Tue Jun 26 2007 Patrick Hartling <patrick@infiscape.com> 0.7.7-2
- Added flagpoll as a requirement for cppdom-devel
- Removed Conflicts directives since this can be installed in parallel with
  other CppDOM releases.

* Thu Nov 09 2006 Patrick Hartling <patrick@infiscape.com>
- Updated for the new world order

* Thu Nov 09 2006 Patrick Hartling <patrick@infiscape.com>
- Fixed bad paths in the .fpc file and in cppdom-config

* Wed Apr 12 2006 Patrick Hartling <patrick@infiscape.com>
- Updated to handle multi-architecture installations.

* Mon Jun 06 2005 Aron Bierbaum <aronb@infiscape.com>
- Total rewrite

* Thu Mar 13 2003 Allen Bierbaum
- First version
