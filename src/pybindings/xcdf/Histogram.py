#!/usr/bin/env python
################################################################################
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# A 1D histogram class that can be filled while looping through a large data
# set.
# 
# Segev BenZvi
################################################################################

__version__ = "$Id$"

import numpy as np

class Histogram:
    """Histogram class that implements fixed and variable binning, and can be
    filled while looping through a large out-of-memory data set.
    """
    def __init__(self, edges=10, xlo=None, xhi=None):
        """Initialize a histogram as follows:

             h = Histogram(n, x0, x1),

           where n is the number of bins and x0, x1 are the histogram limits, or

             h = Histogram(edges),

           where edges define a histogram with variable binning.
           """
        usage = "Histogram(edges | [n], [xlo], [xhi])"
        if isinstance(edges, (int, long)):
            if xlo==None or xhi==None:
                raise Exception(usage)
            self.isFixed = True
            self.n   = int(edges)
            self.xlo = float(xlo)
            self.xhi = float(xhi)
            self.edg = np.linspace(xlo, xhi, self.n+1)
            self.cnt = [0.] * (self.n)
            self.wt2 = [0.] * (self.n)
            self.uf  = 0.
            self.of  = 0.
        elif isinstance(edges, (list, tuple)):
            self.isFixed = False
            self.n   = len(edges)-1
            self.xlo = float(edges[0])
            self.xhi = float(edges[-1])
            self.edg = np.array(edges, dtype=float)
            self.cnt = [0.] * (self.n)
            self.wt2 = [0.] * (self.n)
            self.uf  = 0.
            self.of  = 0.
        else:
            raise Exception(usage)

    def __str__(self):
        """String representation of histogram data: bin edges, counts, and
           uncertainties."""
        hstr = []
        for e0, e1, c, s in zip(self.edg[:-1], self.edg[1:], self.cnt,
        np.sqrt(self.wt2)):
            hstr.append("%12g%12g%12g%12g" % (e0, e1, c, s))
        return "\n".join(hstr)

    def Fill(self, x, w=1.):
        """Fill a histogram with value x and weight w.
        """
        xx = float(x)
        ww = float(w)
        if xx < self.xlo:
            self.uf += w
        elif xx > self.xhi:
            self.of += w
        else:
            if self.isFixed:
                i = int(self.n * (xx - self.xlo)/(self.xhi - self.xlo))
            else:
                i = np.digitize([xx], self.edg) - 1
            self.cnt[i] += w
            self.wt2[i] += w*w

    def Chi2Ndf(self, h):
        """Calculate the chi-square two histograms with weighted counts and the
           same number of bins.
        """
        if self.n != h.n:
            raise ValueError("Number of bins %d != %d" % (self.n, h.n))

        W1 = np.sum(self.cnt)
        W2 = np.sum(h.cnt)
        X2 = 0.
        for i in range(0, self.n):
            w1, v1 = self.cnt[i], self.wt2[i]
            w2, v2 = h.cnt[i], h.wt2[i]
            X2 += (W1*w2 - W2*w1)**2 / (W1**2*v2 + W2**2*v1)

        return X2, self.n-1

    def GetData(self):
        """Return the number of bins, the edges, the bin counts, and the
           uncertainties on the counts.
        """
        return self.n, self.edg, self.cnt, np.sqrt(self.wt2)

