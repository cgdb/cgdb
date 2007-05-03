Instruction for building MacOSX universal binary package
--------------------------------------------------------

  1. Requirements
  
    The latest developer package from apple (http://developer.apple.com) and 
    the MacOSX Version >= 10.4.

  2. Build Structure

    Environment variables:
   
    CGDBDIR = ~/work/cgdb/trunk
    BUILDUR = ~/work/build
    INSTDIR = ~/work/build/usr

    By default cgdb will be installed to /usr/local. 

    In $BUILDIR it exist a link from latest release of readline-5.2 to readline
    and a link to the latest cgdb version.

    e.g.

    % cd $BUILDDIR
    % ls -l

      cgdb -> cgdb-0.6.4
      cgdb-0.6.4
      readline -> readline-5.2
      readline-5.2


  3. Build packages

    Go to $CGDBDIR/packages/mac-pkg folder and run:
    
    % ./build_packages.sh.
  
    As output comes: - a dmg with a mac package inside
                     - a tar ball with the structure on /usr/local
                     - a macosx standard package


  Note: With:
    
        % ./build_packages.sh cleanup 
        
        cane you cleanup the build sources before you build a new version.
    
    
  Jens
