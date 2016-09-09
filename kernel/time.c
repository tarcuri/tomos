unsigned long long int rdtsc()
{
  unsigned long long int x;
  unsigned int a, d;

  asm volatile ("rdtsc" : "=a" (a), "=d" (d));

  return ((unsigned long long)a) | (((unsigned long long)d) << 32);
}
