#!/usr/bin/perl
=head2
Plan: 1. make this pipeline to modular
      2. integrade NOISeq to it
      3. add DE_filter tool for filter sig DEG with different qvalue and fold change
      4. write description about how to use this tool in DEGanalysis pipeline

12/02/2014:add time serious function to edgeR
05/23/2013:combine DESeq and edgeR to one program
06/12/2012:generate pdf for each comparison, including PCA plot, MA plot, and BCV plot.
06/11/2012:identify DE genes using edgeR
=cut

use strict;
use warnings;
use IO::File;
use Getopt::Long;

my $usage = qq'
Perl DEGs_pipeline.pl -s DESeq/edgeR -i raw_count -r rpkm {-a sampleA -b sampleB | -c sampleC} -f 2 -p 0.05 -o output

	-s program (default = DESeq)
	-x function annotation file
	-i raw_count 
	-r rpkm_file 
	-a sampleA
	-b sampleB
	-c sampleList (sampleA,sampleB,sampleC ... )
	-f ratio cutoff
	-p adjust pvalue cutoff
	-o output (default = program)

* the sample name for raw count and rpkm file should be like sample_repN
* the program should be DESeq / edgeR / DESeq2
* perform time course comparison if the comparison has more than 3 samples 

';

my ($help, $program, $raw_count, $rpkm_file, $sample_a, $sample_b, $sample_c, $ratio_cutoff, $padj_cutoff, $paired, $output, $annotation);
GetOptions(
	"h"	=> \$help,
	"s=s"   => \$program,
	"i=s"	=> \$raw_count,
	"r=s"	=> \$rpkm_file,
	"a=s"	=> \$sample_a,
	"b=s"	=> \$sample_b,
	"c=s"	=> \$sample_c,
	"f=s"	=> \$ratio_cutoff,
	"p=s"	=> \$padj_cutoff,
	"x=s"	=> \$annotation,
	"o=s"	=> \$output
);

die $usage if $help;
foreach my $param ( ($program, $raw_count, $rpkm_file, $ratio_cutoff, $padj_cutoff, $output, $annotation) ) {
	die $usage unless $param;
}

#if (defined $paired) {
#	die "Error, can not use $program to perform paired sample analysis\n" unless ( $program eq 'edgeR' || $program eq 'limma');
#}

if ($program eq 'DESeq' || $program eq 'edgeR' || $program eq 'DESeq2') {} else { die "Error in program\n"; }

# convert ratio to foldchange
my $fc1 = $ratio_cutoff; 
my $fc2 = sprintf("%.2f", 1/$ratio_cutoff); 

#================================================================
# store comparison list to hash	
# key: compN; value: sample1 vs sample2				
# file format:
# sample1 \t sample2 \n; --> pairwise comparison
# sample1 \t sample2 \t sample3 \n; --> timeseries comparison
#================================================================

# === put sample_a, sample_b, or sample_c to array
my @comparison;

if (defined $sample_a && defined $sample_b) {
	push(@comparison, "$sample_a\t$sample_b");
}
elsif (defined $sample_c) {
	my @m = split(/,/, $sample_c);
	die "[ERR]less than 3 sample for time series comparison: $sample_c\n" unless (scalar @m >= 3);
	my $sample_c_char = join("\t", @m);
	push(@comparison, $sample_c_char);
}
else {
	print $usage;
	exit;
}

# === do not use this function to store comp_list from file to hash
# @comparison = comparison_to_array($comp_list);

sub comparison_to_array
{
	my $comp_list = shift;
	my @comparison;
	my $fh = IO::File->new($comp_list) || die "Can not open comparison list file $comp_list $!\n";
	while(<$fh>) { 
		chomp; 
		next if $_ =~ m/^#/;
		push(@comparison, $_); 
	}
	$fh->close;
	return @comparison;
}

my @comparisonP = ();
my @comparisonT = ();
foreach my $comp (@comparison) {
	my @samples = split(/\t/, $comp);
	if (scalar(@samples) == 2 ) {
		push (@comparisonP, $comp);
	} elsif ( scalar(@samples) > 2 )  {
		push (@comparisonT, $comp);
	}
}

#================================================================
# load function annotation to hash
# key: gene id; value: ahrd annotation
#================================================================
my %anno;
my $fha = IO::File->new($annotation) || die $!;
while(<$fha>) {
	chomp;
	my @a = split(/\t/, $_);
	$anno{$a[0]} = $a[1];
}
$fha->close;

#================================================================
# parse raw count dataset					
# save comparison data files without zero
# title : key: sample; value: rep1 \t rep2 \t ... repN
# raw: key: gene, sample; value: raw_count1 \t raw_count2 \t ...
#================================================================
my ($title, $raw) = raw_count_to_hash($raw_count);

sub raw_count_to_hash
{
	my $raw_count = shift;

	my %raw; my %title; my ($gene, $sample_name, $sample, $raw_value);

	my $fh = IO::File->new($raw_count) || die "Can not open raw count file $raw_count $!\n";

	# parse raw count title
	my $title = <$fh>; chomp($title);
	my @t = split(/\t/, $title);
	for(my $i=1; $i<@t; $i++)
	{
		$sample_name = $t[$i];
		$sample = $sample_name;
		$sample =~ s/_rep\d+//;
		if (defined $title{$sample} ) {
			$title{$sample}.="\t".$sample_name;
		} else {
			$title{$sample} = $sample_name;
		}
	}

	# parse raw count value
	while(<$fh>)
	{
		chomp;
		my @a = split(/\t/, $_);
		$gene = $a[0];
		for(my $i=1; $i<@a; $i++)
		{
			$sample_name = $t[$i];
			$sample = $sample_name;
			$sample =~ s/_rep\d+//;
			$raw_value = $a[$i];
			if (defined $raw{$gene}{$sample}) {
				$raw{$gene}{$sample} = $raw{$gene}{$sample}."\t".$raw_value;
			} else {
				$raw{$gene}{$sample} = $raw_value;
			}
		}
	}
	$fh->close;

	return(\%title, \%raw);
}

#================================================================
# save comparison data files without zero
# statistics analysis 
#================================================================
my %padj; my %replicate; my %comp_sample;
my $order = 0;
foreach my $comp (@comparison)
{
	# get sample name for each comparison
	my @samples = split(/\t/, $comp);

	# set file name for raw count, statistics analysis output
	$order++;	
	my ($prefix, $raw_file, $out_file, $tmp_file);
	print "[ERR]comparison $comp\n" and exit if ( scalar(@samples) < 2 );
	# $prefix = join("_", @samples) if ( scalar(@samples) == 2 );
	# $prefix = "T".$order if ( scalar(@samples) > 2 );
	$raw_file = $output."_raw";
	$out_file = $output."_out";
    $tmp_file = $output."_tmp";

	# print raw count title
	my $rfh = IO::File->new(">".$raw_file) || die "$raw_file\n";
	print $rfh "gene";
	foreach my $sample (@samples) {
		print $rfh "\t".$$title{$sample};
	}
	print $rfh "\n";

	# print raw count after removing none expressed gene
	foreach my $gene (sort keys %$raw) {
		# remove none expressed gene
		my $sum = 0;
		my $raw_c = $gene;
		foreach my $sample (@samples) {
			$comp_sample{$sample} = 1;	  				# uniq sample name to hash
			my @c = split(/\t/, $$raw{$gene}{$sample});	# get raw count of replicate
			$replicate{$sample} = scalar(@c); 			# get replicate number for sample
			foreach my $c (@c) { $sum = $sum + $c; }	# get total count for one gene 
			$raw_c.="\t".$$raw{$gene}{$sample};			# output line
		}
		print $rfh $raw_c."\n" if $sum > 0;
	}
	$rfh->close;

	# generate R code for comparison
	# contron/treatment comparison
	my $r; my $gene_column; my $pvalue_column;
	if ( scalar(@samples) == 2 )
	{
		# $comp_sample{$sampleA} = 1;
	        # $comp_sample{$sampleB} = 1;

		if ($program eq 'DESeq')
		{
			$r = generate_r_deseq($raw_file, $out_file, $samples[0], $samples[1], $replicate{$samples[0]}, $replicate{$samples[1]});
			$pvalue_column = 8;
			$gene_column = 1;
		}
		elsif ($program eq 'edgeR')
		{
			if (defined $paired) {
				$r = generate_r_edger_pair($raw_file, $out_file, $samples[0], $samples[1], $replicate{$samples[0]}, $replicate{$samples[1]});
				$pvalue_column = 4;
			} else {
				$r = generate_r_edger($raw_file, $out_file, $samples[0], $samples[1], $replicate{$samples[0]}, $replicate{$samples[1]});
				$pvalue_column = 4;
			}
			$gene_column = 0;
		}
		#elsif ($program eq 'limma')
		#{
		#	if (defined $paired) {
		#		$r = generate_r_limma_pair($raw_file, $out_file, $samples[0], $samples[1], $replicate{$samples[0]}, $replicate{$samples[1]});
		#		$pvalue_column = 6;
		#	}
		#	$gene_column = 1;
		#}
		#elsif ($program eq 'VST')
		#{
		#	$r = generate_r_vst($raw_file, $out_file, $samples[0], $samples[1], $replicate{$samples[0]}, $replicate{$samples[1]});
		#	$pvalue_column = 2;
		#	$gene_column = 1;
		#}
		else
		{
			print "[ERR]program $program\n" and exit;
		}
	}
	elsif (@samples > 2)
	{
		#if ($program eq 'limma') 
		#{
		#	$r = generate_r_limma_TS($raw_file, $out_file, \@samples, \%replicate);
		#	$gene_column = 0;
		#	$pvalue_column = -1;
		#}
        if ($program eq 'edgeR')
		{
			print "edgeR code here\n";
			$r = generate_r_edgeR_TS($raw_file, $out_file, \@samples, \%replicate);
			$gene_column = 0;
			$pvalue_column = -1;
		}
		else
		{
			print "[ERR]program $program\n" and exit;
		}
	}

	# perform R code and
	my $tmp = IO::File->new(">$tmp_file") || die "Can not open temp.R file $!\n";
	print $tmp $r;
	$tmp->close;
	system("R --no-save < $tmp_file") && die "Error at cmd R --no-save < temp.R\n";
	
	# parse R output file and save adjusted p value to hash
	my $ofh = IO::File->new($out_file) || die "Can not open DESeq output file $out_file $!\n";
	<$ofh>;
	while(<$ofh>)
	{
		chomp;
		$_ =~ s/"//ig;
		my @a = split(/\t/, $_);
		if (scalar(@samples) == 2) {
			$padj{$a[$gene_column]}{$comp} = $a[$pvalue_column];
		} else {
			my ($c1, $c2) = ($pvalue_column - 2, $pvalue_column - 1);
			$padj{$a[$gene_column]}{$comp} = $a[$c1]."\t".$a[$c2]."\t".$a[$pvalue_column];
		}
		
	}
	$ofh->close;

	# delete temp file
	unlink($tmp_file);
	unlink($raw_file);
	unlink($out_file);
}

#================================================================
# parse RPKM file
# get mean and ratio from RPKM
#================================================================
# total : key: gene_id sample, value: the total expression of all rep for one sample
# rpkm  : key: gene_id sample, value: RPKM for each replictes
my ($total, $RPKM) = rpkm_to_hash($rpkm_file);

sub rpkm_to_hash
{
	my $rpkm_file = shift;

	my $fh = IO::File->new($rpkm_file) || die "Can not open RPKM file $rpkm_file $!\n";

	my %RPKM; my %total; my ($gene, $sample_name, $sample, $RPKM_value);

	# parse title
	my $title_R = <$fh>; chomp($title_R);
	my @tr = split(/\t/, $title_R);

	# parse rpkm value
	while(<$fh>)
	{
		chomp;
		my @a = split(/\t/, $_);
		$gene = $a[0];
		for(my $i=1; $i<@a; $i++)
		{
			$sample_name = $tr[$i]; # sample name
			$sample = $sample_name;
			$sample =~ s/_rep\d+//;
			$RPKM_value = $a[$i];
			if (defined $total{$gene}{$sample})
			{
				$total{$gene}{$sample} = $total{$gene}{$sample} + $RPKM_value;	
			}
			else
			{
				$total{$gene}{$sample} = $RPKM_value;
			}


			if (defined $RPKM{$gene}{$sample})
			{
				$RPKM{$gene}{$sample}.="\t".$RPKM_value;
			}
			else
			{
				$RPKM{$gene}{$sample} = $RPKM_value;
			}
		}
	}
	$fh->close;

	return(\%total, \%RPKM);
}

my %mean;
foreach my $gene (sort keys %$total)
{
	foreach my $sample ( sort keys %{$$total{$gene}} )
	{
		if (defined $comp_sample{$sample})
		{
			my $total = $$total{$gene}{$sample};
			my $rep = $replicate{$sample};
			
			if ($rep == 0) {
				die "Error at replicate num of $sample\n";
			}

			my $mean;
			if ($total > 0) { $mean = $total/$rep; }
			else		{ $mean = 0; }
			$mean{$gene}{$sample} = $mean;
		}
	}
}

my %ratio;  my $low_RPKM;
foreach my $gene (sort keys %mean)
{
	foreach my $comparison (@comparison)
	{
		my @samples = split(/\t/, $comparison);
		if (scalar(@samples) == 2)
		{
			my ($compA, $compB) =  split(/\t/, $comparison);
			my $meanA = $mean{$gene}{$compA};
			my $meanB = $mean{$gene}{$compB};
			my $ratio;
			if (defined $low_RPKM && $low_RPKM > 0 )
			{
				if ($compA == 0 && $compB == 0 )
				{
					$ratio = 1;
				}
				elsif ($compA == 0)
				{
					$ratio = $compB / $low_RPKM;
				}
				elsif ($compB == 0)
				{
					$ratio = $low_RPKM / $compA;
				}
				else
				{
					$ratio = $compB / $compA;
				}
			}
			else
			{
				if ($meanA == 0 && $meanB == 0 )
				{
					$ratio = 1;
				}
				elsif ($meanA == 0)
				{
					$ratio = $meanB / 0.01;
				}
				elsif ($meanB == 0)
				{
					$ratio = 0.01 / $meanA;
				}
				else
				{
					$ratio = $meanB / $meanA;
				}
			}
			$ratio{$gene}{$comparison} = $ratio;
		}
	}
}

#===========================================
# out put result for time series comparison
#===========================================
my $output_all = $output;
if ($output =~ m/deg\.txt/) {
	$output_all =~ s/deg\.txt/deg\.all\.txt/;
}

die "[ERR]output file $output, $output_all\n" if $output_all eq $output;

my $fnum = 0;
foreach my $comp ( @comparisonT )
{
	my @samples = split(/\t/, $comp);
	$fnum++;

	my %p_hash; # save the table to hash according to pvalue; 

	my $out2 = IO::File->new(">".$output_all) || die $!;

	# output title
	my $t = "GeneID\tDescription";
	foreach my $s (@samples) {
		$t.="\t".$$title{$s}."\tmean";
	}
	$t.="\tFDR\n";
	print $out2 $t;

	# output main tables
	my $out_line; my $sig = 0;
	foreach my $gene (sort keys %$RPKM)
	{
		my $function = '';
        $function = $anno{$gene} if defined $anno{$gene};
		$out_line = $gene."\t".$function;

		foreach my $s (@samples) {
			my $mean = $mean{$gene}{$s};
			$mean = sprintf("%.2f", $mean);
			$out_line.="\t".$$RPKM{$gene}{$s}."\t".$mean;
		}

		my $padj = 'NA';	
		if (defined $padj{$gene}{$comp}) {
			my @anova = split(/\t/, $padj{$gene}{$comp});
			$padj = $anova[2];
		} 

		$out_line.="\t".$padj;

		if ($padj ne 'NA' && $padj < $padj_cutoff) {
			$sig++;
			push(@{$p_hash{$padj}}, $out_line);
		}

		print $out2 $out_line."\n";
	}
	$out2->close;

	print "No. of sig (adj p<0.05): $sig for $comp\n";

	# output DEG table for significatnly changed genes
	my $out1 = IO::File->new(">".$output) || die $!;
	print $out1 $t;
	foreach my $p (sort keys %p_hash) {
		my @line = @{$p_hash{$p}};
		foreach my $line (@line) {
			print $out1 $line."\n";
		}
	}
	$out1->close;
}

#================================
# output for pairwise comparison
#================================

if (scalar @comparisonP > 0)
{
	my %p_hash;

	my $out2 = IO::File->new(">".$output_all) || die $!;

	# output title for each comparison
	my $t = "GeneID\tDescription";
	foreach my $comp ( @comparisonP ) {
		my ($sampleA, $sampleB) = split(/\t/, $comp);
		$t.="\t".$$title{$sampleA}."\tmean\t".$$title{$sampleB}."\tmean\tratio\tadjust p";
	}
	print $out2 $t."\n";

	# output rpkm and pvalue for each comparison
	my ($out_line, $sig); 
	my %report; # store number of sig changed gene for each comparison
	foreach my $gene (sort keys %$RPKM)
	{
		my $function = 'unknown';
        $function = $anno{$gene} if defined $anno{$gene};
		$out_line = $gene."\t".$function;

		$sig = 0;
		my $min_padj = 1;
		foreach my $comp ( @comparisonP )
		{
			my ($sampleA, $sampleB) = split(/\t/, $comp);
			my $meanA = $mean{$gene}{$sampleA};
			$meanA = sprintf("%.2f", $meanA);
			my $meanB = $mean{$gene}{$sampleB};
			$meanB = sprintf("%.2f", $meanB);
			my $ratio = $ratio{$gene}{$comp};
			$ratio = sprintf("%.2f", $ratio);

			my $padj = 'NA';
			if (defined $padj{$gene}{$comp}) {
				$padj = $padj{$gene}{$comp};
				$min_padj = $padj if $padj < $min_padj;
				if (($ratio > $fc1 || $ratio < $fc2) && $padj < $padj_cutoff) {
					$sig = 1;
					$report{$comp}++;
				}
			} 

			$out_line.="\t".$$RPKM{$gene}{$sampleA}."\t".$meanA."\t".
						$$RPKM{$gene}{$sampleB}."\t".$meanB."\t".
                        $ratio."\t".$padj;
		}

		if ($sig == 1) {
			push(@{$p_hash{$min_padj}}, $out_line);	
		}

		print $out2 $out_line."\n";
	}
	$out2->close;

    # output DEG table for significatnly changed genes
    my $out1 = IO::File->new(">".$output) || die $!;
	print $out1 $t."\n";
	foreach my $p (sort {$a<=>$b} keys %p_hash) {
		my @line = @{$p_hash{$p}};
		foreach my $line (@line) {
			print $out1 $line."\n";
		}
	}
	$out1->close;
}

# report the number of DE genes for every comparison
# foreach my $comp (sort keys %report)
# {
#	my $num = $report{$comp};
#	$comp =~ s/\s+/ vs /;
#	print $comp."\t".$num."\n";
# }

#================================================================
# kentnf: subroutine for R code
#================================================================
=head2
 generate_r_edgeR_TS -- time series analysis using edgeR
=cut
sub generate_r_edgeR_TS
{
	my ($input, $output, $samples, $replicate ) = @_;

	# get sample numbers including replicates, and construct group
	my ($group, $factor, $design, $comparison) = ('', '', '', '');

	my $num_end = 2; my $k = 0; my $pre_s;
	foreach my $s (@$samples) {
		print "[ERR]do not have replicate numb $s\n" unless defined $$replicate{$s};
		my $rep_num = $$replicate{$s};
		
		$group.= ", rep(\"$s\", $rep_num)";

		$k++;
		for(my $j=0; $j<$rep_num; $j++) { $factor.= $k.","; } # must using number, because the model.matrix will sort by name
		$comparison.= "$s-$pre_s," if $k > 1;
		$design.="\"$s\", ";
		
		$num_end = $num_end + $rep_num;
		$pre_s = $s;
	}

	$group =~ s/^, //;
	$design =~ s/, $//;
	$factor =~ s/,$//;
	$num_end = $num_end - 1;

	# get working folder
	my $pwd = `pwd`;
	chomp($pwd);

	my $r_code = qq'
setwd(\'$pwd\')
library(edgeR)
raw.data<-read.delim("$input", header=TRUE, stringsAsFactors=TRUE)
d <- raw.data[, 2:$num_end]
rownames(d) <- raw.data[, 1]
group <- c($group)
d <- DGEList(counts = d, group = group)
dim(d)
y <- calcNormFactors(d)

# generate design 
design <- model.matrix(~0+factor(c($factor)))
colnames(design) <- c($design)
design

# generate comparison
contrastT <- makeContrasts($comparison levels=design)

# the dispersion has to be estimated
y <- estimateGLMCommonDisp(y,design)
y <- estimateGLMTrendedDisp(y,design)
y <- estimateGLMTagwiseDisp(y,design)

# fit a linear model and test for the treatment ect
fit <- glmFit(y, design)
lrt <- glmLRT(fit, contrast=contrastT)

compTimeF <- topTags(lrt, n=50000, adjust.method="BH")
write.table(compTimeF, sep="\\t", file="$output")

';
	return $r_code;
}

=head2
 generate_r_limma_TS -- time series analysis using limma
=cut
sub generate_r_limma_TS
{
	my ($input, $output, $samples, $replicate ) = @_;

        # get sample numbers including replicates, and construct group
        my ($group, $factor, $design, $comparison) = ('', '', '', '');

        my $num_end = 2; my $k = 0; my $pre_s;
        foreach my $s (@$samples) {
                print "[ERR]do not have replicate numb $s\n" unless defined $$replicate{$s};
                my $rep_num = $$replicate{$s};

                $group.= ", rep(\"$s\", $rep_num)";

                $k++;
                for(my $j=0; $j<$rep_num; $j++) { $factor.= $k.","; } # must using number, because the model.matrix will sort by name
                $comparison.= "$s-$pre_s," if $k > 1;
                $design.="\"$s\", ";

                $num_end = $num_end + $rep_num;
                $pre_s = $s;
        }

        $group =~ s/^, //;
        $design =~ s/, $//;
        $factor =~ s/,$//;
        $num_end = $num_end - 1;

        # get working folder
        my $pwd = `pwd`;
        chomp($pwd);

	my $r_code = qq'
library(DESeq)
library(limma)
countsTable<-read.delim("$input", header=TRUE, stringsAsFactors=TRUE)
rownames(countsTable)<-countsTable\$gene
countsTable<-countsTable[, -1]
conds <- factor( c($group) )
cds<-newCountDataSet(countsTable, conds)
cds <- estimateSizeFactors( cds )
sizeFactors( cds )
cdsBlind <- estimateDispersions( cds, method="blind" )
vsd <- getVarianceStabilizedData( cdsBlind )
eset <-vsd
head(eset)

# generate design 
design <- model.matrix(~0+factor(c($factor)))
colnames(design) <- c($design)

# generate comparison
contrastT <- makeContrasts($comparison levels=design)

# fit dataset to the design
fit <- lmFit(eset, design)
fitTS <- contrasts.fit(fit, contrastT)
fitTS <- eBayes(fitTS)
compTimeF <- topTableF(fitTS, adjust="BH", number=50000)
compTimeF<-compTimeF[,-1]
write.table(compTimeF, sep="\t", file="$output")

';
	return $r_code;
}

=head2
 generate_r_deseq -- 
=cut
sub generate_r_deseq
{
	my ($input, $output, $sampleA, $sampleB, $numA, $numB ) = @_;
	my $pwd = `pwd`;
	chomp($pwd);

	my $DispEsts_pdf = $sampleA."_".$sampleB."_DispEsts.pdf";
	my $DE_pdf = $sampleA."_".$sampleB."_DE.pdf";
	my $hist_pdf = $sampleA."_".$sampleB."_hist.pdf";

	my ($factorA, $factorB);
	for(my $i=0; $i<$numA; $i++) { $factorA.=" \"$sampleA\","; }
	for(my $i=0; $i<$numB; $i++) { $factorB.=" \"$sampleB\","; }
	$factorB =~ s/,$//;
	my $r_code = qq'
setwd(\'$pwd\')
library(DESeq)
countsTable<-read.delim("$input", header=TRUE, stringsAsFactors=TRUE)
rownames(countsTable)<-countsTable\$gene
countsTable<-countsTable[, -1]
conds <- factor( c( $factorA $factorB ) )
cds<-newCountDataSet(countsTable, conds)
cds <- estimateSizeFactors( cds )
cds <- estimateDispersions( cds )

# == plot DispEsts pdf ==
# plotDispEsts <- function( cds )
# {
#   plot(
#   rowMeans( counts( cds, normalized=TRUE ) ),
#   fitInfo(cds)\$perGeneDispEsts,
#   pch = \'.\', log="xy" )
#   xg <- 10^seq( -.5, 5, length.out=300 )
#   lines( xg, fitInfo(cds)\$dispFun( xg ), col="red" )
# }
# pdf("$DispEsts_pdf", width=8, height=6)
# plotDispEsts( cds )

comp <- nbinomTest( cds, "$sampleA", "$sampleB" )
write.table( comp, sep="\\t", file="$output" )

# == plot DE pdf ==
# plotDE <- function( comp )
# plot(
#   comp\$baseMean,
#   comp\$log2FoldChange,
#   log="x", pch=20, cex=.3,
#   col = ifelse( comp\$padj < .05, "red", "black" ) )
# pdf("$DE_pdf", width=8, height=6)
# plotDE( comp )

# == plot histogram pdf ==
# pdf("$hist_pdf", width=8, height=6)
# hist(comp\$pval, breaks=100, col="skyblue", border="slateblue", main="")
';
	return $r_code;	
}

sub generate_r_edger
{
	my ($input, $output, $sampleA, $sampleB, $numA, $numB ) = @_;
	my $num_end = 2+$numA+$numB-1;
	my $pwd = `pwd`;
	chomp($pwd);

	my $pca_pdf = $sampleA."_".$sampleB."_PCA.pdf";
	my $bcv_pdf = $sampleA."_".$sampleB."_BCV.pdf";
	my $ma_pdf = $sampleA."_".$sampleB."_MA.pdf";

	my ($factorA, $factorB);
	for(my $i=0; $i<$numA; $i++) { $factorA.=" \"$sampleA\","; }
	for(my $i=0; $i<$numB; $i++) { $factorB.=" \"$sampleB\","; }	

	$factorB =~ s/,$//;

	my $r_code = qq'
setwd(\'$pwd\')
library(edgeR)
library(limma)
raw.data <- read.delim("$input")
#names(raw.data)

# normalization and filtering
d <- raw.data[, 2:$num_end]
rownames(d) <- raw.data[, 1]
group <- c(rep("$sampleA", $numA), rep("$sampleB", $numB))
d <- DGEList(counts = d, group = group)
dim(d)
cpm.d <- cpm(d)
#d <- d[ rowSums(cpm.d > 1) >=3, ]
d <- calcNormFactors(d)

# == Data exploration, generate PCA pdf ==
# pdf("$pca_pdf",width=8,height=6)
# plotMDS(d, xlim=c(-1,1), labels = c( $factorA $factorB ))

# Estimating the dispersion
d <- estimateCommonDisp(d, verbose=TRUE)
d <- estimateTagwiseDisp(d)
# pdf("$bcv_pdf",width=8,height=6)
# plotBCV(d)
et <- exactTest(d)
result <- topTags(et, n=50000, adjust.method="BH", sort.by="p.value")
write.table( result, sep="\\t", file="$output" )

# == generate MA (Smear) plot ==
# detags <- rownames(topTags(et, n =550000)\$table)
# pdf("$ma_pdf",width=8,height=6)
# plotSmear(et, de.tags=detags)
# abline(h = c(-2, 2), col = "dodgerblue")
';
	return $r_code;
}

=head2

 generate R code for paired samples used for edgeR

=cut
sub generate_r_edger_pair
{
        my ($input, $output, $sampleA, $sampleB, $numA, $numB ) = @_;
        my $num_end = 2+$numA+$numB-1;
        my $pwd = `pwd`;
        chomp($pwd);

        my $pca_pdf = $sampleA."_".$sampleB."_PCA.pdf";
        my $bcv_pdf = $sampleA."_".$sampleB."_BCV.pdf";
        my $ma_pdf = $sampleA."_".$sampleB."_MA.pdf";

	die "Error for number of comparison samples\n" if $numA != $numB;

	my $subject = "";
	for(my $i=1; $i<=$numA; $i++) { $subject.= "\"S$i\", "; }
	$subject = $subject.$subject;
	$subject =~ s/, $//;

	my $treatment = "";
	for(my $i=0; $i<$numA; $i++) { $treatment.= "\"C\", "; }
	for(my $i=0; $i<$numB; $i++) { $treatment.= "\"T\", "; }
	$treatment =~ s/, $//;

        my ($factorA, $factorB);
        for(my $i=0; $i<$numA; $i++) { $factorA.=" \"$sampleA\","; }
        for(my $i=0; $i<$numB; $i++) { $factorB.=" \"$sampleB\","; }

        $factorB =~ s/,$//;

        my $r_code = qq'
setwd(\'$pwd\')
library(edgeR)

# load dataset
raw.data <- read.delim("$input")
d <- raw.data[, 2:$num_end]
rownames(d) <- raw.data[, 1]

# normalization and filtering
# group <- c(rep("$sampleA", $numA), rep("$sampleB", $numB))
d <- DGEList(counts = d)
dim(d)
#cpm.d <- cpm(d)
#d <- d[ rowSums(cpm.d > 1) >=3, ]
d <- calcNormFactors(d)

# design matrix
subject <- factor(c($subject))
treatment <- factor(c($treatment), levels=c("C", "T"))
design <- model.matrix(~subject+treatment)
design


# Data exploration, generate PCA pdf
# pdf("$pca_pdf",width=8,height=6)
# plotMDS(d)

# Estimating the dispersion
d <- estimateGLMCommonDisp(d, design, verbose=TRUE)
d <- estimateGLMTrendedDisp(d, design)
d <- estimateGLMTagwiseDisp(d, design)

# pdf("$bcv_pdf",width=8,height=6)
# plotBCV(d)

fit <- glmFit(d, design)
lrt <- glmLRT(fit)
result <- topTags(lrt, n=50000, adjust.method="BH", sort.by="p.value")
write.table( result, sep="\\t", file="$output" )

# generate MA (Smear) plot
# detags <- rownames(topTags(lrt, n = 50000)\$table)
# pdf("$ma_pdf", width=8, height=6)
# plotSmear(lrt, de.tags=detags)
# abline(h = c(-2, 2), col = "dodgerblue")
';
        return $r_code;
}

