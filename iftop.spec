Summary:	iftop - provides realtime bandwith usage on an interface
Name:		iftop
Version:	0.8
Release:	1
License:	GPL
Group:		Applications/Internet
Source0:	http://www.ex-parrot.com/~pdw/iftop/download/iftop-0.8.tar.gz
URL:		http://www.ex-parrot.com/~pdw/iftop/
Buildroot:	%{_tmppath}/%{name}-%{version}-%(id -u -n)
Packager:	Riku Meskanen <mesrik@cc.jyu.fi>
BuildRequires:	libpcap
BuildRequires:	ncurses-devel > 5.2-12

%description
iftop does for network usage what top(1) does for CPU usage. It listens 
to network traffic on a named interface and displays a table of current
bandwidth usage by pairs of hosts. Handy for answering the question 
"why is our ADSL link so slow?"

%prep
%setup -q

%build

make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT iftop
gzip -9 iftop.8
mkdir -p ${RPM_BUILD_ROOT}%{_sbindir} ${RPM_BUILD_ROOT}%{_mandir}/man8
install -m 755 iftop ${RPM_BUILD_ROOT}%{_sbindir}
install -m 644 iftop.8.gz ${RPM_BUILD_ROOT}%{_mandir}/man8

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README CHANGES COPYING TODO INSTALL
%attr(755,root,root) %{_sbindir}/iftop
%{_mandir}/man8/*

%changelog
* Fri Aug 30 2002 Iain Lea <iain@bricbrac.de>
- Updated build for Red Hat 7.3 version
* Sat Apr 13 2002 Riku Meskanen <mesrik@cc.jyu.fi>
- Initial build for Red Hat 7.2 version

