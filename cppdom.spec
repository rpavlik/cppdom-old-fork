#
# Spec file for cppdom
#rpmbuild -bb -v --define='_topdir /var/tmp/cppdom' --define='_rpmdir /var/tmp/cppdom' --buildroot=/..../cppdom/build.linux/dist/cppdom-0.5.3 cppdom.spec
# Run as: 
#
%define _name cppdom
%define lib_name %{_name}

Summary: A C++ XML API for use with STL
Name: %{_name}
Version: 0.7.0
Release: 1
License: LGPL
Group: Development/Libraries
Source0: %{_name}-%{version}.tar.gz
Prefix: /usr
URL: http://www.sf.net/projects/xml-cppdom/
BuildRequires: python scons
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Vendor: xml-cppdom Project
Packager: Infiscape Corporation
Provides: libcppdom = %{version}-%{release} %{_name} = %{version}-%{release}

%description
CppDOM is a C++ based XML loader and writer with an internal DOM
representation. It is lightweight and high-performance. The goal of the
project is to provide a lightweight C++ interface for XML programming that
is similar to the API and functionality of JDOM.

%package -n %{lib_name}-devel
Summary: Header files and libraries needed for %{_name} development
Group: Development/C++
Requires: %{lib_name} = %{version}-%{release}
Provides: libcppdom-devel = %{version}-%{release}
Conflicts: libcppdom-devel < %{version}-%{release}

%description -n %{lib_name}-devel
The header files and libraries needed for developing programs using CppDOM.

%prep
%setup -n %{_name}-%{version}

%build
lib_subdir=`echo %{_libdir} | sed -e "s|%{_prefix}/\(.*\)|\1|"`
CXXFLAGS="$RPM_OPT_FLAGS"
LINKFLAGS="$RPM_OPT_FLAGS"
export CXXFLAGS
export LINKFLAGS
scons prefix=%{_prefix} libdir=$lib_subdir optimize=yes

%install
[ -z %{buildroot} ] || rm -rf %{buildroot}

lib_subdir=`echo %{_libdir} | sed -e "s|%{_prefix}/\(.*\)|\1|"`
CXXFLAGS="$RPM_OPT_FLAGS"
LINKFLAGS="$RPM_OPT_FLAGS" 
export CXXFLAGS
export LINKFLAGS
%{?makeinstall_std:%makeinstall_std}%{!?makeinstall_std:scons prefix=%{buildroot}%{_prefix} libdir=$lib_subdir install}
find %{buildroot}%{_prefix} -name .sconsign -exec rm {} \;

%clean
[ -z %{buildroot} ] || rm -rf %{buildroot}

%post -n %{lib_name}
/sbin/ldconfig

%postun -n %{lib_name}
/sbin/ldconfig

%files -n %{lib_name}
%defattr(-, root, root)
%{_libdir}/*.so
%{_prefix}/test/*

%files -n %{lib_name}-devel
%defattr(-, root, root)
%{_bindir}/cppdom-config
%{_includedir}/cppdom/*.h
%{_includedir}/cppdom/ext/*.h
%{_libdir}/*.a

%changelog
* Wed Apr 12 2006 Patrick Hartling
- Total rewrite.

* Thu Mar 13 2003 Allen Bierbaum
- First version
