%define version  0.4.2
%define name     naev
%define dataname %{name}-data
%define release  %mkrel 1

Name: %{name}
Version: %{version}
Release: %{release}
Summary: 2D space trading and combat game
Group: Games/Arcade 
License: GPLv3+
URL: http://code.google.com/p/naev/

Source0: http://naev.googlecode.com/files/%{name}-%{version}.tar.bz2
Source1: naev.png
Source2: http://naev.googlecode.com/files/ndata-%{version}
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Requires: SDL >= 1.2
Requires: openal
BuildRequires: SDL-devel
BuildRequires: libxml2-devel
BuildRequires: freetype2-devel
BuildRequires: libpng-devel
BuildRequires: libopenal-devel
BuildRequires: libvorbis-devel >= 1.2.1
BuildRequires: binutils

%description
NAEV is a 2D space trading and combat game, taking inspiration from the
Escape Velocity series.

You pilot a space ship from a top-down perspective, and are more or less
free to do what you want. As the genre name implies, you’re able to trade
and engage in combat at will. Beyond that, there’s an ever-growing number
of storyline missions, equipment, and ships; Even the galaxy itself grows
larger with each release. For the literarily-inclined, there are large
amounts of lore accompanying everything from planets to equipment.

%package -n %{dataname}
Group: Games/Arcade
License: GPLv3+ AND GPLv3 AND GPLv2+ AND Public Domain AND CC-by 3.0 AND CC-by-sa 3.0
Summary: Data files for %{name}
Requires: naev

%description -n %{dataname}
NAEV is a 2D space trading and combat game, taking inspiration from the
Escape Velocity series.

This is the data file.

%prep
%setup -q

%build
%configure2_5x --with-ndata-path=/usr/share/naev/ndata-%{version}
# the bfd debugging tool is bugged under mandriva <= 2010.2 and need to be
# called by hand.
%make LIBS=-liberty

%install
rm -rf %{buildroot}
%makeinstall_std
install -m644 %{SOURCE1} -D %{buildroot}%{_iconsdir}/naev32.png
install -m644 %{SOURCE2} -D %{buildroot}%{_datadir}/naev/ndata-%{version}
mkdir -p %{buildroot}/usr/share/applications/
%__cat << EOF > %{buildroot}%{_datadir}/applications/%{name}.desktop
[Desktop Entry]
Name=Naev
GenericName=Naev
Comment=2D space trading and combat game
Icon=%{_iconsdir}/naev32.png
Exec=%{_bindir}/%{name}
Type=Application
Categories=Game;StrategyGame;
StartupNotify=true
EOF

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc README LICENSE AUTHORS TODO
%attr(755,root,root) %{_bindir}/naev

%{_mandir}/man6/naev.6.*
%{_datadir}/applications/naev.desktop
%{_iconsdir}/naev32.png

%files -n %{dataname}
%defattr(-,root,root)
%{_datadir}/naev/ndata-%{version}

%changelog
* Mon Feb 14 2011 Ludovic Bellière <xrogaan@gmail.com> 0.4.2-1
- Cleanup spec.
- Created with base on spec from blogdrake.net

