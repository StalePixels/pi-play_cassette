
double tzx_dutycycle;                   // SampleFreq/CPU Speed

int tzx_ticks_to_samples(int n)
{
  return ((int) (0.5 + (tzx_dutycycle * (double) n)));
}