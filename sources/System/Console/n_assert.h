#ifndef _N_ASSERT_H_
#define _N_ASSERT_H_

#ifdef _DEBUG
# define NAssert(exp) (void)( (exp) || (__NAssert(#exp, __FILE__, __LINE__), 0) )
void __NAssert(const char *, const char *, unsigned);
#else
# define NAssert(exp)
#endif

#define NCodeMissing NAssert(0);
#define NInvalid NAssert(0);
#define NImplies(a,b) if(a) NAssert(b);

#endif
