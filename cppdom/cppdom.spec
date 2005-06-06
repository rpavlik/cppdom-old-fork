# Spec file for cppdom.
%define name    cppdom
%define version	0.3.3
%define release	1

Name: %{name}
Summary: A C++ based XML loader and writer with an internal DOM representation.
Version: %{version}
Release: %{release}
Source: %{name}-%{version}.tar.gz
URL: http://sourceforge.net/projects/xml-cppdom
Group: System Environment/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
License: LGPL
BuildPrereq: scons >= 0.96.1
Provides: cppdom = %{version}-%{release}
Obsoletes: cppdom <= %{version}-%{release}

%description
A C++ based XML loader and writer with an internal DOM representation. It is
very lightweight and high-performance. The goal of this project is to provide
a lightweight C++ interface for XML programming that is similar in API and
functionality to JDOM.

%package devel
Summary: The CppDOM Headers
Group: System Environment/Libraries
Requires: cppdom = %{version}-%{release}
Provides: cppdom-devel = %{version}-%{release}

%description devel
Headers for CppDOM

%prep
rm -rf $RPM_BUILD_ROOT
%setup -q

%build
scons optimize=yes prefix=$RPM_BUILD_ROOT/usr

%install
scons install prefix=$RPM_BUILD_ROOT/usr
# Remove all stupid scons temp files
find $RPM_BUILD_ROOT/usr -name .sconsign -exec rm {} \;

%clean
rm -rf $RPM_BUILD_ROOT

%pre

%post

%preun

%postun

%files
%defattr(-, root, root)
/usr/bin/cppdom-config
/usr/lib/*.so

%files devel
%defattr(-, root, root)
/usr/include/cppdom/*.h
/usr/lib/*.a

%doc README AUTHORS ChangeLog COPYING

%changelog
