/* Copyright 2017 Lime Microsystems Ltd.Licensed under the Apache License, Version 2.0 (the "License");you may not use this file except in compliance with the License.You may obtain a copy of the License at    http://www.apache.org/licenses/LICENSE-2.0Unless required by applicable law or agreed to in writing, softwaredistributed under the License is distributed on an "AS IS" BASIS,WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.See the License for the specific language governing permissions andlimitations under the License.*/// tukey cooley algorithm, verified against Octave// size of data objects optimised for computation speed#include <iostream>#include <fstream>#include <cmath>#include <complex>#include <complex.h>#include "FFTlib.hpp"using namespace std;// taking bit b of the index and multiplying this by the truncated reverse of the indexlong	CTpow( long index, unsigned char bit, unsigned char ndigits ){	long	temp=0;	long	origpos=1;	long	newpos=1<<(ndigits-1);	long	endpos=newpos<<1;	for( unsigned char i=0; i<ndigits; i++ )	{		if( ((index & origpos)>0) ) // inline for speed			temp |= newpos;			newpos>>=1;		origpos<<=1;	}			return( (1<<bit)*(temp % (endpos>>bit)) );}// Window to handle frequencies which are not harmonics of the sample rate// Hann window is the most effective for our work// Hann 0.5 + 0.5 * cos( )// Hamming 0.54 + 0.46 * cos( )// Kaiser is based on modified bessel functions of the first kind// Jo( b*sqrt(1-(n/N)^2) )/Jo(b) 4<=b<=9// See numerical recipes for Bartlett and Welsh windowsvoid	Window( float _Complex vec[], long N ){	for( long ci=0; ci<N; ci++ )		vec[ci] *= 1.0 - cos( (2*ci)*(_PI/N) );}// N must be an integer power of 2^n// data contains N points data[0]-data[N]// spectrum is initialised spectrum[0]-spectrum[N]void	FFT( float _Complex data[], float _Complex spectrum[], long N ){	signed char	log2N=(int)floor( log10( N+0.0001 )/log10( 2.0 ) ); // log2() not defined in C++!  Improvise		for( long i=0; i<N; i++ ) // copy complex time domain data into complex spectrum vector		spectrum[i] = data[i];	long		ra=0;	long		rb=0;	long		f=0;	signed char	b=0;	long		origpos=0;	long		newpos=0;	unsigned char	ci=0;	signed char	log2Nm1=log2N-1;	float _Complex	t2;	float _Complex	*W = new float _Complex[N];	float _Complex	arg = cexpf( -I*2*_PI/N ); // define look up table of W^p	W[0] = 1.0;	W[1] = arg; // smallest rotation in fft	for( long i=2; i<N; i++ )		W[i] = W[i-1] * arg;	long bitpos=(1<<n-1);	for( b=log2Nm1; b>=0; b-- ) // bit we are operating on	{		for( f=0; f<N; f++ )		{			ra = f & ~bitpos; // mask f b 0			if( f == ra ) // only do once - it's a pair			{				rb = f | bitpos; // mask f b 1				if(vec[ rb ]!=0.0) // avoid '0.0' maths computations				{					t2 = spectrum[ rb ];					spectrum[ rb ] = spectrum[ ra ] + t2 * W[CTpow( rb, b, n )];					spectrum[ ra ] = spectrum[ ra ] + t2 * W[CTpow( ra, b, n )];				}				else					spectrum[ rb ] = spectrum[ ra ];			}		}		bitpos>>=1;	}		for( ra=0; ra<N; ra++ ) // ra=f reorder frequencies and normalise	{		rb = 0; ;		origpos=1;		newpos=1<<log2N;		for( ci=0; ci<log2N; ci++ ) // RB( ra, log2N ) - inline for speed		{			newpos>>=1;			if( ((ra & origpos)>0) )				rb |= newpos;				origpos<<=1;		}		if( ra != rb ) // something to do		{			if( ra < rb ) // only do this once or we will be back where we started			{				t2 = spectrum[ rb ];				spectrum[ rb ] = spectrum[ ra ] / (float)N;				spectrum[ ra ] = t2 / (float)N;			}		}		else // palendromic - nothing to do			spectrum[ra] /= (float)N;	}	delete [] W;}// N must be an integer power of 2^n// spectrum is initialised spectrum[0]-spectrum[N]// positive frequencies 0-N/2-1 negative frequencies N-2/N// data contains N points data[0]-data[N]// although real, we have to use complex to carry out calculationvoid	InvFFT( float _Complex spectrum[], float _Complex data[], long N ){	signed char	log2N=(int)floor( log10( N+0.0001 )/log10( 2.0 ) ); // log2() not defined in C++!  Improvise	for( long i=0; i<N; i++ )		 data[i] = spectrum[i];	long		ra=0;	long		rb=0;	long		f=0;	signed char	b=0;	long		origpos=0;	long		newpos=0;	unsigned char	ci=0;	double _Complex	t2;	signed char 	log2Nm1=log2N-1;	float _Complex	*W = new float _Complex[N];	float _Complex	arg = cexpf( I*2*_PI/N ); // reverse exponent sign for inv fft	W[0] = 1.0;	W[1] = arg; // smallest rotation in fft	for( long i=2; i<N; i++ ) // define look up table of W^p		W[i] = W[i-1] * arg;	long bitpos=(1<<n-1);	for( b=log2Nm1; b>=0; b-- ) // bit we are operating on	{		for( f=0; f<N; f++ ) // pairs of numbers - need temporary vars as orig overwritten 		{			ra = f & ~bitpos; // mask f b 0			rb = f | bitpos; // mask f b 1			if( f == ra ) // only do once - it's a pair			{				rb = f | bitpos; // mask b 1				if(vec[ rb ]!=0.0) // avoid '0.0' maths computations				{					t2 = data[ rb ];					data[ rb ] = data[ ra ] + t2 * W[CTpow( rb, b, n )];					data[ ra ] = data[ ra ] + t2 * W[CTpow( ra, b, n )];				}				else					data[ rb ] = data[ ra ];			}		}		bitpos>>=1;	}		for( long ra=0; ra<N; ra++ ) // ra=f reorder frequencies and normalise	{		rb = 0; ;		origpos=1;		newpos=1<<log2N;		for( ci=0; ci<log2N; ci++ ) // RB( ra, log2N ) - inline for speed		{			newpos>>=1;			if( ((ra & origpos)>0) )				rb |= newpos;				origpos<<=1;		}		if( ra != rb ) // something to do		{			if( ra < rb ) // only do this once or we will be back where we started			{ // convention vary, but we normalise fft, but not ifft				t2 = data[ rb ];				data[ rb ] = data[ra]; // / (float)N;				data[ ra ] = t2; // / (float)N;			}		}//		else // palendromic - nothing to do//			data[ra] /= (float)N;	}	delete [] W;}void conj( float _Complex vec[], long N ){	for( long ci=0; ci<N; ci++ )		vec[ci]=conj(vec[ci]);}void dotprod( float _Complex vecr[], float _Complex vec1[], float _Complex vec2[], long N ){	for( long ci=0; ci<N; ci++ )		vecr[ci]=vec1[ci]*vec2[ci];}long maxloc(float _Complex vec[], long N){	int loc=0;	float mx=0;	for( long ci=0; ci<N; ci++ )		if( cabsf(vec[ci])>mx )		{			loc=ci;			mx=cabsf(vec[ci]);		}	return(loc);}void cshift( float _Complex seq1[], float _Complex seq2[], long N, long off ){	long idx;	for( long ci=0; ci<N; ci++ )	{		idx=(ci+off)%N;		seq2[idx]=seq1[ci];	}}void cpyvec( float _Complex seq1[], float _Complex seq2[], long N ){	for( long ci=0; ci<N; ci++ )		seq2[ci]=seq1[ci];}void acf( float _Complex seq1[], float _Complex seq2[], long N, long *pos, float *mg, float *ph ){	float _Complex	*seq1fft=new float _Complex[N];	float _Complex	*seq2fft=new float _Complex[N];	float _Complex	*res=new float _Complex[N];	float _Complex	*acfv=new float _Complex[N];	FFT(seq1,seq1fft,N);	FFT(seq2,seq2fft,N);	conj(seq2fft,N);	dotprod(res,seq1fft,seq2fft,N);	InvFFT(res,acfv,N);	(*pos)=maxloc(acfv,N);	(*mg)=cabsf(acfv[(*pos)]);	(*ph)=cargf(acfv[(*pos)]);	delete [] acfv;	delete [] res;	delete [] seq2fft;	delete [] seq1fft;}// remove unwanted duplicate conj-fft for increased speedvoid acf2( float _Complex seq1[], float _Complex seq2conjfft[], long N, long *pos, float *mg, float *ph ){	float _Complex	*seq1fft=new float _Complex[N];	float _Complex	*res=new float _Complex[N];	float _Complex	*acfv=new float _Complex[N];	FFT(seq1,seq1fft,N);	dotprod(res,seq1fft,seq2conjfft,N);	InvFFT(res,acfv,N);	(*pos)=maxloc(acfv,N);	(*mg)=cabsf(acfv[(*pos)]);	(*ph)=cargf(acfv[(*pos)]);	delete [] acfv;	delete [] res;	delete [] seq1fft;}