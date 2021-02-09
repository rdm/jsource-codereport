/* Copyright 1990-2011, Jsoftware Inc.  All rights reserved.               */
/* Licensed use only. Any other use is in violation of copyright.          */
/*                                                                         */
/* Verbs: Primes and Factoring                                             */

#include "j.h"


#define MM   25000L     /* interval size to look for primes        */
#define PMAX 105097564L /* upper limit of p: ; (_1+2^31) = p: PMAX */
#define PT   500000L    /* interval size in ptt                    */


static const I ptt[]={
    7368791L,   15485867L,   23879539L,   32452867L,   41161751L,
   49979693L,   58886033L,   67867979L,   76918277L,   86028157L,
   95189093L,  104395303L,  113648413L,  122949829L,  132276713L,
  141650963L,  151048973L,  160481219L,  169941223L,  179424691L, /*  10e6 */
  188943817L,  198491329L,  208055047L,  217645199L,  227254213L,
  236887699L,  246534487L,  256203221L,  265892021L,  275604547L,
  285335587L,  295075153L,  304836293L,  314606891L,  324407131L,
  334214467L,  344032391L,  353868019L,  363720403L,  373587911L, /*  20e6 */
  383446691L,  393342743L,  403245719L,  413158523L,  423087253L,
  433024253L,  442967117L,  452930477L,  462900953L,  472882049L,
  482873137L,  492876863L,  502895647L,  512927377L,  522960533L,
  533000401L,  543052501L,  553105253L,  563178743L,  573259433L, /*  30e6 */
  583345003L,  593441861L,  603538541L,  613651369L,  623781269L,
  633910111L,  644047709L,  654188429L,  664338817L,  674506111L,
  684681301L,  694847539L,  705031199L,  715225741L,  725420411L,
  735632797L,  745843009L,  756065179L,  766301759L,  776531419L, /*  40e6 */
  786760649L,  797003437L,  807247109L,  817504253L,  827772511L,
  838041647L,  848321921L,  858599509L,  868891399L,  879190841L,
  889495223L,  899809363L,  910112683L,  920419823L,  930754037L,
  941083987L,  951421147L,  961748941L,  972092467L,  982451707L, /*  50e6 */
  992801861L, 1003162837L, 1013526181L, 1023893887L, 1034271001L,
 1044645419L, 1055040229L, 1065433427L, 1075824283L, 1086218501L,
 1096621151L, 1107029839L, 1117444369L, 1127870683L, 1138305547L,
 1148739817L, 1159168537L, 1169604841L, 1180041943L, 1190494771L, /*  60e6 */
 1200949609L, 1211405387L, 1221863261L, 1232332813L, 1242809783L,
 1253270833L, 1263751141L, 1274224999L, 1284710771L, 1295202523L,
 1305698249L, 1316196209L, 1326697579L, 1337195527L, 1347701867L,
 1358208613L, 1368724913L, 1379256029L, 1389786323L, 1400305369L, /*  70e6 */
 1410844907L, 1421376533L, 1431916091L, 1442469313L, 1453010737L,
 1463555011L, 1474118929L, 1484670179L, 1495213271L, 1505776963L,
 1516351777L, 1526922017L, 1537493917L, 1548074371L, 1558655507L,
 1569250363L, 1579833509L, 1590425983L, 1601020433L, 1611623887L, /*  80e6 */
 1622223991L, 1632828059L, 1643429663L, 1654054511L, 1664674819L,
 1675293223L, 1685912299L, 1696528907L, 1707155683L, 1717783153L,
 1728417367L, 1739062387L, 1749701927L, 1760341447L, 1770989611L,
 1781636627L, 1792287229L, 1802933621L, 1813593029L, 1824261419L, /*  90e6 */
 1834925117L, 1845587717L, 1856264467L, 1866941123L, 1877619461L,
 1888303063L, 1898979371L, 1909662913L, 1920354661L, 1931045239L,
 1941743653L, 1952429177L, 1963130473L, 1973828669L, 1984525423L,
 1995230821L, 2005933283L, 2016634099L, 2027354369L, 2038074751L, /* 100e6 */
 2048795527L, 2059519673L, 2070248617L, 2080975187L, 2091702673L,
 2102429887L, 2113154951L, 2123895979L, 2134651583L, 2145390539L,
};   /* p: PT*1+i.210 */

#define ptn (I)(sizeof(ptt)/sizeof(ptt[0]))

static I jtsup(J jt,I n,I*wv){I c,d,j,k;
 c=0; DO(n, j=wv[i]; ASSERT(0<=j,EVDOMAIN); if(c<j)c=j;);
 ASSERT(c<=PMAX,EVLIMIT);
 j=1; k=0; DQ(128, if(c<=j)break; j+=j; ++k;); d=c*k;
 return k&&c>d?IMAX:MAX(d,135L);
}    /* (_1+2^31) <. 135 >. (*>.@(2&^.)) >./ w */

static void sieve(I n,I m,B*b,B*u){I i,j,q;
 static const B t[]={
  0,1,0,0,0, 0,0,1,0,0, 0,1,0,1,0, 0,0,1,0,1, 0,0,0,1,0, 0,0,0,0,1,
  0,1,0,0,0, 0,0,1,0,0, 0,1,0,1,0, 0,0,1,0,1, 0,0,0,1,0, 0,0,0,0,1};
 mvc(m,b,30L,(C*)t+n%30); if(!n)b[1]=0; q=1+(I)sqrt(n+(D)m);
 if(n)for(i=7;i<q;i+=2){if(u[i]){j=n%i; j=j?i-j:0; while(j<m){b[j]=0; j+=i;}}}
 else for(i=7;i<q;i+=2){if(b[i]){j=i+i;            while(j<m){b[j]=0; j+=i;}}}
}    /* sieve b for n+i.m, but if 0=n then b=. 0 (2 3 5)}b */

static A jtprime1(J jt, A w){A d,t,y,z;B*b,*u;I c,*dv,e,i,j,k,m,n,p,q,*wv,x,*zv;
 if(!w) return 0;
 k=0; n=AN(w); wv=AV(w); RE(m=jtsup(jt,n,wv)); RESETRANK; JBREAK0;
 GATV(z,INT,n,AR(w),AS(w)); zv= AV(z);
 RZ(d=grade1(ravel(w)));  dv= AV(d);
 if(jt->p4792){I*u=AV(jt->p4792); c=AN(jt->p4792); while(n>k&&c>(x=wv[dv[k]]))zv[dv[k++]]=u[x];}
 else{
  while(n>k&&0==wv[dv[k]])zv[dv[k++]]=2;
  while(n>k&&1==wv[dv[k]])zv[dv[k++]]=3;
  while(n>k&&2==wv[dv[k]])zv[dv[k++]]=5;
 }
 if(n==k){return z;}
 j=3; p=0; e=PT; q=1+(I)sqrt((D)m); x=wv[dv[k]];
 GATV0(t,B01,q,1);         u=BAV(t); sieve(0L,q,u,u);
 GATV0(y,B01,MIN(m,MM),1); b=BAV(y);
 for(;BETWEENO(p,0,m);p+=q){
  if(x>=e){c=x/PT; e=PT*(1+c); c=MIN(c,ptn); if(j<c*PT){j=c*PT; p=ptt[c-1];}}
  JBREAK0; q=MIN(MM,m-p); sieve(p,q,b,u); c=j+q/3;
  if(x>c)for(i=1-(p&1);i<q;i+=2)j+=b[i];
  else   for(i=1-(p&1);i<q;i+=2)
   if(b[i]){while(j==x){zv[dv[k++]]=i+p; if(n==k)return z; x=wv[dv[k]];} ++j;}
 }
 while(n>k)zv[dv[k++]]=p; return z;
}

static I init4792(J jt) {
 if(!jt->p4792){RZ(jt->p4792=prime1(IX(4792L))); ACX(jt->p4792);} return 1;
}

static A jtprime1d(J jt, A w){A d,z;D*wv,x,*zv;I*dv,k,n;
 if(!w) return 0;
 n=AN(w); wv=DAV(w);
 GATV(z,FL,n,AR(w),AS(w)); zv=DAV(z);
 RZ(d=grade1(ravel(w))); dv=AV(d);
 k=0; while(n>k&&(D)PMAX>=wv[dv[k]])++k;
 if(k){A y;I*yv;
  RZ(y=prime1(jtcvt(jt,INT,jtfrom(jt,jttake(jt,sc(k),d),ravel(w))))); yv=AV(y);
  DO(k, zv[dv[i]]=(D)*yv++;);
 }
 if(k==n)return z;
 DO(n-k, x=wv[dv[i]]; ASSERT(0<=x&&x!=inf,EVDOMAIN););
 ASSERT(0,EVLIMIT);
}

 A jtprime(J jt, A w){PROLOG(0061);A z;B b=1;I n,t;
 RZ(init4792(jt));
 n=AN(w); t=AT(w);
 if(!(t&INT))RZ(w=jtpcvt(jt,INT,w));
 if(INT&AT(w)){
  // if the maximum in the argument is <= PMAX, call prime1.  Force minimum of interval to <=0
  // so that full range compares against PMAX
  if(condrange(AV(w),n,0,IMIN,PMAX).range){b=0; RZ(z=prime1(w));}
 }
 if(b)RZ(z=prime1d(FL&AT(w)?w:jtcvt(jt,FL,w)));
 if(t&XNUM+RAT)RZ(z=jtcvt(jt,XNUM,z));
 EPILOG(z);
}    /* p:"r w */

static I jtsuq(J jt,I n,I*wv){I c=24; DO(n, c=MAX(c,wv[i]););  return c==0x7fffffff?c:1+c;}
     /* 1+24>.>./w */

 A jtplt(J jt, A w){PROLOG(0062);A d,t,y,z;B*b,*u,xt;I c,*dv,e,i,j,k,m,n,p,q,*wv,x,*zv;
 xt=1&&AT(w)&XNUM+RAT;
 if(!(INT&AT(w)))RZ(w=vi(ceil1(w))); wv=AV(w); JBREAK0;
 j=3; k=p=c=0; e=*ptt; n=AN(w);
 RE(m=jtsuq(jt,n,wv)); ASSERT(m<=0x7fffffff,EVLIMIT); q=1+(I)sqrt((D)m);
 GATV0(t,B01,q,1);         u =BAV(t); sieve(0L,q,u,u);
 GATV0(y,B01,MIN(m,MM),1); b =BAV(y);
 GATV(z,INT,n,AR(w),AS(w)); zv= AV(z);
 RZ(d=grade1(ravel(w)));  dv= AV(d);
 while(n>k&&2>=wv[dv[k]])zv[dv[k++]]=0;
 while(n>k&&3>=wv[dv[k]])zv[dv[k++]]=1;
 while(n>k&&5>=wv[dv[k]])zv[dv[k++]]=2;
 if(n==k){EPILOG(z);} x=wv[dv[k]];
 for(;BETWEENO(p,0,m);p+=q){
  if(x>=e){
   while(ptn>c&&x>=ptt[c])++c;
   if(j<c*PT){p=ptt[c-1]; e=c<ptn?ptt[c]:IMAX; j=c*PT;}
  }
  JBREAK0; q=MIN(MM,m-p); sieve(p,q,b,u);
  if(x>p+q)
   for(i=1-(p&1);i<q;i+=2)j+=b[i];
  else
   for(i=1-(p&1);i<q;i+=2)if(b[i]){
    while(x<=p+i){zv[dv[k++]]=j; if(n==k){i=q; break;} x=wv[dv[k]];}
    ++j;
 }}
 while(n>k)zv[dv[k++]]=j;
 if(xt)RZ(z=jtcvt(jt,XNUM,z));
 EPILOG(z);
}    /* p:^:_1 w, the number of primes less than w */


static B jtxprimeq(J,I,X);
static A jtxprimetest(J jt, A w);

static const B pmsk[]={0,0,1,1,0,1,0,1,0,0, 0,1,0,1,0,0,0,1,0,1, 0,0,0,1,0,0,0,0,0,1, 0,1};
     /* indicates which i<32 is prime */

static A jtiprimetest(J jt, A w){A z;B*b;I d,j,n,*pv,q,*v,wn,*wv;
 wn=AN(w); wv=AV(w); pv=AV(jt->p4792);
 DO(wn, if(2147483647L<wv[i])return xprimetest(jtcvt(jt,XNUM,w)););
 GATV(z,B01,wn,AR(w),AS(w)); b=BAV(z);
 for(j=0;j<wn;++j){
  n=*wv++; v=pv;
  if(32>n)b[j]=pmsk[MAX(0,n)];
  else{b[j]=1; DQ(AN(jt->p4792), d=*v++; q=n/d; if(n==q*d){b[j]=0; break;}else if(q<d)break;);}
 }
 return z;
}

static A jtxprimetest(J jt, A w){A z;B*b,rat;I d,j,q,n,*pv,*v,wn,wt,*yv;X r,*wv,x,xmaxint,y;
 wn=AN(w); wt=AT(w); wv=XAV(w); pv=AV(jt->p4792);
 rat=1&&wt&RAT; RZ(xmaxint=xc(2147483647L)); RZ(y=xc(-1L)); yv=AV(y);
 GATV(z,B01,wn,AR(w),AS(w)); b=BAV(z);
 for(j=0;j<wn;++j){
  x=*wv++; d=AV(x)[AN(x)-1]; b[j]=1; v=pv;
  if(rat&&jtxcompare(jt,iv1,*wv++)){b[j]=0; continue;}
  ASSERT(d!=XPINF&&d!=XNINF,EVDOMAIN);
  if(0>=d)b[j]=0;
  else if(1==jtxcompare(jt,x,xmaxint)){
   A *old=jt->tnextpushp;
   DQ(100, *yv=*v++; RZ(r=jtxrem(jt,y,x)); if(!AV(r)[0]){b[j]=0; break;});
   if(b[j])RE(b[j]=jtxprimeq(jt,100L,x));
   tpop(old);
  }else{
   n=xint(x); v=pv;
   if(32>n)b[j]=pmsk[MAX(0,n)];
   else DQ(AN(jt->p4792), d=*v++; q=n/d; if(n==q*d){b[j]=0; break;}else if(q<d)break;);
 }}
 return z;
}    /* prime test for extended integers or rationals */

static A jtprimetest(J jt, A w){A x;I t;
 t=AT(w);
 if((UI)SGNIF(t,B01X)>=(UI)AN(w))return jtreshape(jt,shape(jt,w),num(0));  // AN is 0, or t is boolean
 switch(CTTZ(t)){
  default:             ASSERT(0,EVDOMAIN);
  case INTX:            return iprimetest(w);
  case RATX: case XNUMX: return xprimetest(w);
  case FLX:  case CMPXX:
   {PUSHCCT(1.0-FUZZ) x=eq(t&FL?w:conjug(w),floor1(w)); POPCCT}
   return xprimetest(jtcvt(jt,XNUM,tymes(w,x)));
}}   /* primality test */


static A jtnextprime(J jt, A w){A b,fs,x,y;B*bv;I k,n,*xv,*yv;X*wv;
 n=AN(w);
 if((UI)SGNIF(AT(w),B01X)>=(UI)AN(w))return jtreshape(jt,shape(jt,w),num(2));
 ASSERT(NUMERIC&AT(w),EVDOMAIN);
 RZ(fs=eval("2&+^:(0&p:)^:_"));
 GATV(x,INT,n,AR(w),AS(w)); xv=AV(x);
 if(INT&AT(w)){B b=1;I*wv=AV(w);
  DQ(n, k=*wv++; if(k==IMAX){b=0; break;}else *xv++=2>k?2:(k+1)|1;);
  if(b)return rank1ex0(x,fs,FAV(fs)->valencefns[0]);
  RZ(w=jtcvt(jt,XNUM,w));
 }
 if(AT(w)&FL+RAT)RZ(w=jtcvt(jt,XNUM,floor1(       w )));
 if(AT(w)&CMPX  )RZ(w=jtcvt(jt,XNUM,floor1(jtcvt(jt,FL,w))));
 GATV(b,B01,n,AR(w),AS(w)); bv=BAV(b); wv=XAV(w);
 DQ(n, y=*wv++; yv=AV(y); *bv++=0<yv[AN(y)-1]; k=*yv; *xv++=AN(y)==1&&2>k?2-k:(k&1)+1; );
 return rank1ex0(tymes(b,plus(w,x)),fs,FAV(fs)->valencefns[0]);
}

static A jtprevprime(J jt, A w){A fs,x,y;I k,m,n,*xv,*yv;X*wv;
 n=AN(w);
 ASSERT(!n||NUMERIC&AT(w)&&!(B01&AT(w)),EVDOMAIN);
 RZ(fs=eval("_2&+^:(0&p:)^:_"));
 GATV(x,INT,n,AR(w),AS(w)); xv=AV(x);
 if(INT&AT(w)){I*wv=AV(w);
  DQ(n, k=*wv++; ASSERT(2<k,EVDOMAIN); *xv++=3==k?2:(k-2)|1;);
  return rank1ex0(x,fs,FAV(fs)->valencefns[0]);
 }
 if(AT(w)&FL+RAT)RZ(w=jtcvt(jt,XNUM,ceil1(       w )));
 if(AT(w)&CMPX  )RZ(w=jtcvt(jt,XNUM,ceil1(jtcvt(jt,FL,w))));
 wv=XAV(w);
 DQ(n, y=*wv++; yv=AV(y); m=AN(y); k=*yv; ASSERT(0<yv[m-1]&&(1<m||2<k),EVDOMAIN); *xv++=1==m&&3==k?1:1+(k&1););
 return rank1ex0(minus(w,x),fs,FAV(fs)->valencefns[0]);
}

static A jttotient(J jt, A w){A b,x,z;B*bv,p=0;I k,n,t;
 n=AN(w); t=AT(w);
 if(t&B01)return w;
 GATV(b,B01,n,AR(w),AS(w)); bv=BAV(b);
 if(t&INT){I*wv=AV(w),*xv;
  GATV(x,INT,n,AR(w),AS(w)); xv=AV(x);
  DQ(n, k=*wv++; ASSERT(0<=k,EVDOMAIN); if(k){*bv++=1; *xv++=k;}else{*bv++=0; *xv++=1; p=1;};);
 }else{X*xv,y;
  RZ(x=jtcvt(jt,XNUM,w)); xv=XAV(x);
  DO(n, y=xv[i]; k=AV(y)[AN(y)-1]; ASSERT(0<=k,EVDOMAIN); if(k)*bv++=1; else{*bv++=0; xv[i]=iv1; p=1;});
 }
 A z0; z=jtcvt(jt,AT(x),df1(z0,x,eval("(- ~:)&.q:")));
 return p?tymesW(b,z):z;
}

/*
MillerRabin=: 100&$: : (4 : 0) " 0
 if. 0=2|y do. 2=y return. end.
 if. 74>y do. y e. i.&.(p:^:_1) 74 return. end.
 e=. huo y-1
 for_a. x witnesses y do. if. (+./c=y-1) +: 1={:c=. a y&|@^ e do. 0 return. end. end.
 1
)
*/

static B jtspspd(J jt,I b,I n,I d,I h){D a,n1,nn,x;
 if(b==n)return 1;
 a=1; x=(D)b; nn=(D)n; n1=(D)n-1;
 while(d){if(1&d)a=fmod(a*x,nn); x=fmod(x*x,nn); d>>=1;}
 if(a==1||a==n1)return 1;
 DQ(h-1, a=fmod(a*a,nn); if(a==n1)return 1;);
 return 0;
}

static B jtspspx(J jt,I b,I n,I d,I h){I ai,n1;X a,ox,xn;
 if(b==n)return 1;
 n1=n-1;
 ox=jt->xmod; jt->xmod=jtcvt(jt,XNUM,sc(n)); a=jtxpow(jt,xc(b),xc(d)); jt->xmod=ox;
 ai=xint(a);
 if(ai==1||ai==n1)return 1;
 xn=xc(n);
 DQ(h-1, a=jtxrem(jt,xn,jtxtymes(jt,a,a)); if(xint(a)==n1)return 1;);
 return 0;
}

static A jtdetmr(J jt, A w){A z;B*zv;I d,h,i,n,wn,*wv;
 RZ(w=vi(w));
 wn=AN(w); wv=AV(w);
 GATV(z,B01,wn,AR(w),AS(w)); zv=BAV(z);
 for(i=0;i<wn;++i){
  n=*wv++;
  if(1>=n||!(1&n)||0==n%3||0==n%5){*zv++=0; continue;}
  h=0; d=n-1; while(!(1&d)){++h; d>>=1;}
  if     (n< 9080191)*zv++=spspd(31,n,d,h)&&spspd(73,n,d,h);
  else if(n<94906266)*zv++=spspd(2 ,n,d,h)&&spspd( 7,n,d,h)&&spspd(61,n,d,h);
  else               *zv++=spspx(2 ,n,d,h)&&spspx( 7,n,d,h)&&spspx(61,n,d,h);
 }
 RE(0); return z;
}    /* deterministic Miller-Rabin */

 A jtpco2(J jt,A a,A w){A z;B*b;I k;
 RZ(init4792(jt));
 RE(k=i0(a));
 switch(k){
  default: ASSERT(0,EVDOMAIN);
  case -4: return prevprime(w);
  case -1: return plt(w);
  case  0: RZ(z=primetest(w)); b=BAV(z); DQ(AN(z), *b=!*b; ++b;); return z;
  case  1: return primetest(w);
  case  2: return jtqco2(jt,scf(infm),w);
  case  3: return factor(w);
  case  4: return nextprime(w);
  case  5: return totient(w);
  case  6: return detmr(w);
}}   /* a p: w */

static A jtqco2x(J jt,I m,A w){A y;I c,*dv,i,*pv,*yv;X d,q,r,x;
 RZ(init4792(jt));
 if(!(XNUM&AT(w)))RZ(w=jtcvt(jt,XNUM,w));
 x=XAV(w)[0]; pv=AV(jt->p4792); RZ(d=xc(2L)); dv=AV(d);
 GATV0(y,INT,m,1); yv=AV(y); memset(yv,C0,m*SZI);
 for(i=0;i<m;++i){
  c=0; *dv=pv[i];
  while(1){RZ(xdivrem(x,d,&q,&r)); if(AV(r)[0])break; ++c; x=q;}
  yv[i]=c; if(1==AN(x)&&1==AV(x)[0])break;
 }
 return jtcvt(jt,XNUM,y);
}    /* m q: w where 0<:m and p: m is one xdigit and w is a single extended integer */

 A jtqco2(J jt,A a,A w){A q,y,z;B b,bb,xt;I c,j,k,m,*qv,wn,wr,*yv,*zv;
 wn=AN(w); wr=AR(w); b=all1(lt(a,zeroionei(0))); xt=1&&AT(w)&XNUM+RAT;
 if(AR(a)||wr&&(b||xt))return rank2ex0(a,w,UNUSED_VALUE,jtqco2);
 if(!b&&xt){RE(m=i0(vib(a))); if(BETWEENO(m,0,1229))return jtqco2x(jt,m,w);}  /* 1229=p:^:_1 XBASE */
 RZ(q=factor(w)); qv=AV(q);
 if(b)RZ(a=negate(a));
 bb=jtequ(jt,a,ainf);
 if(b&bb){ /* __ q: w */
  RZ(y=ne(q,curtail(jtover(jt,zeroionei(0),q))));
  return jtlamin2(jt,jtrepeat(jt,y,q),df1(z,y,jtcut(jt,ds(CPOUND),zeroionei(1))));
 }
 RZ(y=vi(plt(q))); yv=AV(y);
 k=-1; DO(AN(y), if(k<yv[i])k=yv[i];); ++k;
 if(bb)m=k; else RE(m=i0(a));
 if(b){
  q=jtrepeat(jt,ge(y,sc(k-m)),q);
  return jtlamin2(jt,nub(q),df2(z,q,q,sldot(ds(CPOUND))));
 }else{
  GATV(z,INT,wn*m,1+wr,AS(w)); AS(z)[wr]=m; zv=AV(z);
  memset(zv,C0,AN(z)*SZI);
  j=0; c=AS(q)[wr]; DQ(wn, DQ(c, if(qv[j]&&m>yv[j])++zv[yv[j]]; ++j;); zv+=m;);
  return AT(w)&XNUM+RAT?jtcvt(jt,XNUM,z):z;
}}   /* a q: w for array w */

 A jtfactor(J jt, A w){PROLOG(0063);A y,z;I c,d,i,k,m,n,q,*u,*v,wn,*wv,*zv;
 RZ(init4792(jt));
 if(AT(w)&FL+CMPX){
  RZ(y=jtpcvt(jt,INT,w));
  if(INT&AT(y))w=y;
 }
 RZ(w=vi(w));
 wn=AN(w); wv=AV(w);
 n=0; DO(wn, k=wv[i]; ASSERT(0<k,EVDOMAIN); n=MAX(n,k););
 u=AV(jt->p4792); c=8*SZI-2;
 GATV(z,INT,c*wn,1+AR(w),AS(w)); AS(z)[AR(w)]=c; v=zv=AV(z);
 for(i=m=0;i<wn;++i){
  n=*wv++;
  DO(AN(jt->p4792), d=u[i]; q=n/d; while(n==q*d){*v++=d; n=q; q/=d;} if(q<d)break;);
  if(1<n)*v++=n;
  d=v-zv; m=MAX(m,d); zv+=c; while(v<zv)*v++=0;
 }
 z=c==m?z:jttaker(jt,m,z);
 EPILOG(z);
}    /* q:"r w */

/* if 0=n xprimeq y, then y is certainly composite; and                    */
/* if 1=n xprimeq y, then y is prime with a probability of error of 0.25^n */

static B jtxprimeq(J jt,I n,X y){A h,om=jt->xmod;B b;I*dv,i,k,*pv;X d,m,t,x,y1;
 ASSERT(n<=AN(jt->p4792),EVLIMIT);
 pv=AV(jt->p4792);
 GAT0(h,XNUM,1,0); XAV(h)[0]=y; jt->xmod=h;
 k=0; RZ(t=xc(2L)); RZ(m=y1=jtxminus(jt,y,iv1));
 while(0==(AV(m)[0]&1)){++k; RZ(m=xdiv(m,t,XMFLR));}
 GAT0(d,INT,1,1); dv=AV(d);  // could use faux block
 A *old=jt->tnextpushp;
 for(i=0;i<n;++i){
  dv[0]=pv[i]; RZ(x=jtxpow(jt,d,m)); b=1==AN(x)&&1==AV(x)[0];
  DQ(k*!b, if(!jtxcompare(jt,x,y1)){b=1; break;} RZ(x=jtxrem(jt,y,xsq(x))););
  tpop(old);
  if(!b)break;
 }
 jt->xmod=om; return b;
}    /* y assumed to be not in n{.jt->p4792 */
