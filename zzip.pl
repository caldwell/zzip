 #!/usr/bin/perl -w
use Crypt::Blowfish;
use Compress::Zlib;

my $version = "2.3";

my %header;

my @opt_extra_headers;
my $opt_compression = 1;
my $opt_zzip = 1;
my $opt_dos_file_names;
my ($opt_encryption_index, $opt_encryption_file);
my ($opt_input_file, $opt_output_file);
for(@ARGV) {
       if (/^-h(.*)/)  { push @opt_extra_header,$1 }
    elsif (/^-s/)      { $opt_dos_file_names = 1 }
    elsif (/^-Z/)      { $opt_compression = 0 }
    elsif (/^-v/)      { print "$0 version 1.0 ($version)\n"; exit 0 }
    elsif (/^-e(\d+)/) { $opt_encryption_index = $1 }
    elsif (/^-E(.*)/)  { $opt_encryption_file = $1 }
    elsif (/^-u/)      { $opt_zzip = 0 }
    elsif (/^-p/)      { $header{'Image Type'} = "Gem Preferences" }
    elsif (/^-f/)      { $header{'Image Type'} = "Firmware" }
    elsif (/^-F/)      { $header{'Image Type'} = "File" }
    elsif (/^-L/)      { $header{'File Type'} = "large" }
    elsif (/^-M/)      { $header{'File Type'} = "medium" }
    elsif (/^-S/)      { $header{'File Type'} = "small" }
    elsif (/^-d(.*)/)  { $header{'Destination Path'} = $1 }
    elsif (/^-o(.*)/)  { $opt_output_file = $1 }
    elsif (/^-(.)/)    { die "'$1' is an invalid option.\n" }
    else  {
        die "Only one file may be specified; can't compress \"$opt_input_file\" AND \"$_\".\n"
            if $opt_input_file;
        $opt_input_file = $_;
	}
}

die "Looks like you forgot the ",($opt_encryption_index?"-E":"-e"), "command line option:\n"
    if defined $opt_encryption_file xor defined $opt_encryption_index and not $ENV{ZZIP_KEY};

@ARGV = $opt_input_file ? ($opt_input_file) : ();
{ local $/; $data = <>; }

print STDERR "data: $data";

$header{'Uncompressed Length'} = sprintf "0x%08X", length $data;

$data = compress($data) if $opt_compression;

$header{'Compression'}         = $opt_compression ? "zlib" : "none";
$header{'Checksum'}            = sprintf "0x%08X", unpack "%32C*", $data;
$header{'CRC'}                 = sprintf "0x%08X", Compress::Zlib::crc32($data);

print STDERR "zdata: $data";
my $edata = $data;
if ($opt_encryption_index || $ENV{ZZIP_KEY}) {
	my $key = ParseKeyFile($opt_encryption_file, $opt_encryption_index) if $opt_encryption_index;
	$key = $ENV{ZZIP_KEY} unless $key;
	print STDERR "key: ", length $key, "\n";
    $edata = encrypt($key, $data);
    $header{'Unencrypted Length'} = sprintf "0x%08X", length $data;
    $header{'Encryption'}         = "blowfish";
    $header{'Encryption Key'}     = $opt_encryption_index;
    $header{'Plaintext Checksum'} = sprintf "0x%08X", unpack "%32C*", $data;
    $header{'Plaintext CRC'}      = sprintf "0x%08X", Compress::Zlib::crc32($data);
	$data = $edata;
}
print STDERR "edata: $edata";
$header{'Compressed Length'}   = sprintf "0x%08X", length $edata; # should really be called "Encrypted Length" (only on encryption)

$header{'Date'}                 = localtime;
$header{'Name'}                 = $opt_input_file if $opt_input_file;

if (!$opt_output_file && $opt_input_file) {
	$opt_output_file = $opt_input_file;
	$opt_output_file =~ s/\.[^.]$// if $opt_dos_file_names;
	$opt_output_file .= ".z";
} 

open STDOUT, ">$opt_output_file" if $opt_output_file;
print "zzip version 1.0 ($version)\n";
my $max_length = (sort { $a <=> $b } map { length } keys %header)[-1];
for (sort keys %header) {
    printf "%-${max_length}s = $header{$_}\n",$_;
}
print map { "$_\n" } @opt_extra_header;
print "\0", $edata;

sub encrypt {
    my ($key,$data) = @_;
    my $pad = 8 - length($data) % 8;
    print STDERR "Old length: ",length $data," ";
    $data .= chr($pad) x $pad; # standard padding (see Crypt::CBC)
#   $data .= '\0' x (8 - length $data % 8) if length $data;
    print STDERR "New length: ",length $data,"\n";
    my $cipher = new Crypt::Blowfish $key;
    my $offset = 0;
    while ($offset < length $data) {
		print STDERR "offset $offset\n";
        substr($data,$offset,8) = $cipher->encrypt(substr($data,$offset,8));
        $offset+= 8;
    }
    return $data;
}

sub ParseKeyFile {
	my ($file, $index) = @_;
	open FILE, "<", $file or die;
	<FILE> =~ /^Version 2 Keyfile:/ or die;
	while (local $_ = <FILE>) {
		chomp;
		/^(\d\d) (.{64})$/ or die;
		return $2 if $1 == $index;
	}
	die "No Key $index in $file\n";
}
