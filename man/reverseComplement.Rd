\name{reverseComplement}

\alias{reverse,MaskedXString-method}
\alias{complement}
\alias{complement,DNAString-method}
\alias{complement,RNAString-method}
\alias{complement,DNAStringSet-method}
\alias{complement,RNAStringSet-method}
\alias{complement,XStringViews-method}
\alias{complement,MaskedDNAString-method}
\alias{complement,MaskedRNAString-method}
\alias{reverseComplement}
\alias{reverseComplement,DNAString-method}
\alias{reverseComplement,RNAString-method}
\alias{reverseComplement,DNAStringSet-method}
\alias{reverseComplement,RNAStringSet-method}
\alias{reverseComplement,XStringViews-method}
\alias{reverseComplement,MaskedDNAString-method}
\alias{reverseComplement,MaskedRNAString-method}


\title{Sequence reversing and complementing}

\description{
  Use these functions for reversing sequences and/or complementing DNA
  or RNA sequences.
}

\usage{
complement(x, \dots)
reverseComplement(x, \dots)
}

\arguments{
  \item{x}{
    A \link{DNAString}, \link{RNAString},
    \link{DNAStringSet}, \link{RNAStringSet},
    \link{XStringViews} (with \link{DNAString} or \link{RNAString} subject),
    \link{MaskedDNAString} or \link{MaskedRNAString} object
    for \code{complement} and \code{reverseComplement}.
  }
  \item{\dots}{
    Additional arguments to be passed to or from methods.
  }
}

\details{
  See \code{?\link[IRanges]{reverse}} for reversing an \link{XString},
  \link{XStringSet} or \link{XStringViews} object.

  If \code{x} is a \link{DNAString} or \link{RNAString} object,
  \code{complement(x)} returns an object where each base in \code{x}
  is "complemented" i.e. A, C, G, T in a \link{DNAString} object are replaced
  by T, G, C, A respectively and A, C, G, U in a \link{RNAString} object
  are replaced by U, G, C, A respectively.

  Letters belonging to the IUPAC Extended Genetic Alphabet are also
  replaced by their complement (M <-> K, R <-> Y, S <-> S, V <-> B,
  W <-> W, H <-> D, N <-> N) and the gap (\code{"-"}) and hard masking
  (\code{"+"}) letters are unchanged.

  \code{reverseComplement(x)} is equivalent to \code{reverse(complement(x))}
  but is faster and more memory efficient.
}

\value{
  An object of the same class and length as the original object.
}

\seealso{
  \link[IRanges]{reverse},
  \link{DNAString-class},
  \link{RNAString-class},
  \link{DNAStringSet-class},
  \link{RNAStringSet-class},
  \link{XStringViews-class},
  \link{MaskedXString-class},
  \code{\link{chartr}},
  \code{\link{findPalindromes}},
  \code{\link{IUPAC_CODE_MAP}}
}

\examples{
## ---------------------------------------------------------------------
## A. SOME SIMPLE EXAMPLES
## ---------------------------------------------------------------------

x <- DNAString("ACGT-YN-")
reverseComplement(x)

library(drosophila2probe)
probes <- DNAStringSet(drosophila2probe)
probes
alphabetFrequency(probes, collapse=TRUE)
rcprobes <- reverseComplement(probes)
rcprobes
alphabetFrequency(rcprobes, collapse=TRUE)

## ---------------------------------------------------------------------
## B. OBTAINING THE MISMATCH PROBES OF A CHIP
## ---------------------------------------------------------------------

pm2mm <- function(probes)
{
    probes <- DNAStringSet(probes)
    subseq(probes, start=13, end=13) <- complement(subseq(probes, start=13, end=13))
    probes
}
mmprobes <- pm2mm(probes)
mmprobes
alphabetFrequency(mmprobes, collapse=TRUE)

## ---------------------------------------------------------------------
## C. SEARCHING THE MINUS STRAND OF A CHROMOSOME
## ---------------------------------------------------------------------
## Applying reverseComplement() to the pattern before calling
## matchPattern() is the recommended way of searching hits on the
## minus strand of a chromosome.

library(BSgenome.Dmelanogaster.UCSC.dm3)
chrX <- Dmelanogaster$chrX
pattern <- DNAString("ACCAACNNGGTTG")
matchPattern(pattern, chrX, fixed=FALSE)  # 3 hits on strand +
rcpattern <- reverseComplement(pattern)
rcpattern
m0 <- matchPattern(rcpattern, chrX, fixed=FALSE)
m0  # 5 hits on strand -

## Applying reverseComplement() to the subject instead of the pattern is not
## a good idea for 2 reasons:
## (1) Chromosome sequences are generally big and sometimes very big
##     so computing the reverse complement of the positive strand will
##     take time and memory proportional to its length.
chrXminus <- reverseComplement(chrX)  # needs to allocate 22M of memory!
chrXminus
## (2) Chromosome locations are generally given relatively to the positive
##     strand, even for features located in the negative strand, so after
##     doing this:
m1 <- matchPattern(pattern, chrXminus, fixed=FALSE)
##     the start/end of the matches are now relative to the negative strand.
##     You need to apply reverseComplement() again on the result if you want
##     them to be relative to the positive strand:
m2 <- reverseComplement(m1)  # allocates 22M of memory, again!
##     and finally to apply rev() to sort the matches from left to right
##     (5'3' direction) like in m0:
m3 <- rev(m2) # same as m0, finally!

## WARNING: Before you try the example below on human chromosome 1, be aware
## that it will require the allocation of about 500Mb of memory!
if (interactive()) {
  library(BSgenome.Hsapiens.UCSC.hg18)
  chr1 <- Hsapiens$chr1
  matchPattern(pattern, reverseComplement(chr1))  # DON'T DO THIS!
  matchPattern(reverseComplement(pattern), chr1)  # DO THIS INSTEAD
}
}

\keyword{methods}
\keyword{manip}
