# Spec file for cppdom.
%define name    cppdom
%define version 1.0.0
%define release 1

Name: %{name}
Summary: A C++ based XML loader and writer with an internal DOM representation.
Version: %{version}
Release: %{release}%{?dist}
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

%package config
Summary: The cppdom-config script that calls flagpoll
Group: Development/C++
Requires: cppdom-devel
Requires: flagpoll >= 0.8.1
Requires: python

%description config
The cppdom-config Python script that provides backwards compability for older
packages not using Flagpoll for getting compiler options necessary for
building against CppDOM.

%package doc
Summary: CppDOM documentation
Group: Development/C++
BuildPrereq: doxygen
BuildPrereq: graphviz

%description doc
CppDOM API documentation in HTML form.

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
cd doc && doxygen cppdom.doxy
cd ..

%install
[ -z %{buildroot} ] || rm -rf %{buildroot}
CXXFLAGS="$RPM_OPT_FLAGS"
LINKFLAGS="$RPM_OPT_FLAGS" 
export CXXFLAGS
export LINKFLAGS
scons prefix=%{buildroot}%{_prefix} var_arch=%{arch} var_type=optimized build_test=no install
mkdir -p %{buildroot}%{_docdir}/cppdom-%{version}
mv doc/html %{buildroot}%{_docdir}/cppdom-%{version}
# Remove all stupid scons temp files
find %{buildroot}%{_prefix} -name .sconsign -exec rm {} \;
for f in README AUTHORS ChangeLog COPYING ; do
   cp $f %{buildroot}%{_docdir}/cppdom-%{version}
done

%clean
[ -z %{buildroot} ] || rm -rf %{buildroot}

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-, root, root)
%{_libdir}/*.so
%dir %{_docdir}/cppdom-%{version}/
%doc %{_docdir}/cppdom-%{version}/README
%doc %{_docdir}/cppdom-%{version}/AUTHORS
%doc %{_docdir}/cppdom-%{version}/ChangeLog
%doc %{_docdir}/cppdom-%{version}/COPYING

%files devel
%defattr(-, root, root)
%dir %{_includedir}/cppdom-%{version}/
%dir %{_includedir}/cppdom-%{version}/cppdom/
%{_includedir}/cppdom-%{version}/cppdom/*.h
%dir %{_includedir}/cppdom-%{version}/cppdom/ext/
%{_includedir}/cppdom-%{version}/cppdom/ext/*.h
%{_libdir}/*.a
%{_libdir}/flagpoll

%files config
%defattr(-, root, root)
%{_bindir}/cppdom-config

%files doc
%defattr(-, root, root)
%dir %{_docdir}/cppdom-%{version}/
%doc %{_docdir}/cppdom-%{version}/html

%changelog
* Mon Mar 02 2009 Patrick Hartling <patrick.hartling@priority5.com> 1.0.0-1
- Updated to version 1.0.0.

* Wed Aug 01 2007 Patrick Hartling <patrick@infiscape.com> 0.7.10-1
- Updated to version 0.7.10.

* Fri Jul 06 2007 Patrick Hartling <patrick@infiscape.com> 0.7.9-1
- Updated to version 0.7.9.

* Wed Jun 27 2007 Patrick Hartling <patrick@infiscape.com> 0.7.8-2
- Removed cppdom-config from the cppdom-devel package.

* Tue Jun 26 2007 Patrick Hartling <patrick@infiscape.com> 0.7.8-1
- Updated to version 0.7.8.
- Include rendered API documentation in the new cppdom-doc package.
- Package cppdom-config separately so that multiple versions of the cppdom
  package can be installed in parallel.

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
