# A utility that writes the header file helptext.h that contains the README file.

$LINE_NUMBER = 0;
open ( FILE, "../../README" ) or die "Can't open file for reading ../../README";
open ( OUT,  ">../include/helptext.h" ) or die "Can't open file for writing helptext.h" ;

print OUT "#ifndef __HELPTEXT_H__\n";
print OUT "#define __HELPTEXT_H__\n";
print OUT "\n";
print OUT "static char *cgdb_help_text[] = {\n";

while ( <FILE> ) {
    $LINE_NUMBER++;
    chomp $_;
# Check to see if the length is greater than 80 chars and warn
    $l = length ( $_ );
    s/\"/\\\"/g;
    printf "WARNING: Length exceeded 80 chars ($l) at line $LINE_NUMBER\n" if $l > 80;
    printf OUT "\"$_\",\n";
}

print OUT "NULL\n";
print OUT "};\n";
print OUT "\n";
print OUT "#endif /* __HELPTEXT_H__ */\n";
