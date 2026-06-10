#include <stdio.h>
#include <stddef.h>

typedef struct {
  const char *title;
  const char *summary;
  const char *evidence_target;
  const char *tags[8];
  size_t tag_count;
} project_profile_t;

static const project_profile_t profile = {
  "Multi-Sensor Data Logger over I2C/SPI",
  "Multi-threaded C/C++ Linux data logger that reads temperature, pressure, and humidity sensors through /dev/i2c-* or /dev/spidev* and stores records in SQLite.",
  "Linux file-based peripheral access, concurrent sensor reads, local persistence, and repeatable data-capture evidence.",
  {
  "I2C",
  "SPI",
  "POSIX threads",
  "SQLite",
  "C/C++",
  "Sensor logging"
  },
  6u
};

int main(void) {
  printf("%s\n", profile.title);
  printf("Summary: %s\n", profile.summary);
  printf("Evidence target: %s\n", profile.evidence_target);
  printf("Stack:");

  for (size_t index = 0; index < profile.tag_count; ++index) {
    printf(" %s%s", profile.tags[index], index + 1u == profile.tag_count ? "" : ",");
  }

  printf("\n");
  return 0;
}
