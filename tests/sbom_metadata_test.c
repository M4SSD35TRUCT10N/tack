#include "tack_test_embed.h"

static int file_contains_text(const char *path, const char *needle) {
  FILE *f;
  char buf[8192];
  size_t n;
  if (!path || !needle) return 0;
  f = fopen(path, "rb");
  if (!f) return 0;
  n = fread(buf, 1, sizeof(buf) - 1, f);
  fclose(f);
  buf[n] = '\0';
  return strstr(buf, needle) != 0;
}

static int write_file_from_writer(const char *path, void (*writer)(FILE*, const SbomData*), const SbomData *d) {
  FILE *f = fopen(path, "wb");
  if (!f) return 1;
  writer(f, d);
  if (fclose(f) != 0) return 1;
  return 0;
}

int main(void) {
  int failures = 0;
  Target t;
  char name_buf[] = "app";
  char id_buf[] = "app";
  char src_buf[] = "src";
  char bin_buf[] = "tack";
  StrVec includes, defines, cflags, ldflags, libs, srcs, core_srcs;
  SbomData d;

  memset(&t, 0, sizeof(t));
  t.name = name_buf;
  t.id = id_buf;
  t.src_dir = src_buf;
  t.bin_base = bin_buf;
  t.enabled = 1;

  sv_init(&includes); sv_push(&includes, "include");
  sv_init(&defines);  sv_push(&defines, "DEBUG=1");
  sv_init(&cflags);   sv_push(&cflags, "-g");
  sv_init(&ldflags);  sv_init(&libs);
  sv_init(&srcs);     sv_push(&srcs, "src/tack.c");
  sv_init(&core_srcs);

  memset(&d, 0, sizeof(d));
  d.t = &t;
  d.profile = "debug";
  d.compiler = "gcc";
  d.config_enabled = 1;
  d.config_path = "tack.ini";
  d.tackfile_mode = "none";
  d.use_core_effective = 0;
  d.includes = &includes;
  d.defines = &defines;
  d.cflags = &cflags;
  d.ldflags = &ldflags;
  d.libs = &libs;
  d.srcs = &srcs;
  d.core_srcs = &core_srcs;
  d.sbom_spec_version = "1.4";

  ensure_dir_recursive("build");

  if (write_file_from_writer("build/sbom-metadata.cdx.json", write_sbom_cyclonedx, &d) != 0) {
    fprintf(stderr, "write cyclonedx failed\n");
    return 1;
  }

  d.sbom_spec_version = "2.3";
  if (write_file_from_writer("build/sbom-metadata.spdx.json", write_sbom_spdx, &d) != 0) {
    fprintf(stderr, "write spdx failed\n");
    return 1;
  }

  if (!file_contains_text("build/sbom-metadata.cdx.json", "\"serialNumber\": \"urn:uuid:")) {
    fprintf(stderr, "missing CycloneDX serialNumber\n");
    failures++;
  }
  if (!file_contains_text("build/sbom-metadata.cdx.json", "\"timestamp\": \"20")) {
    fprintf(stderr, "missing CycloneDX timestamp\n");
    failures++;
  }
  if (!file_contains_text("build/sbom-metadata.spdx.json", "\"created\": \"20")) {
    fprintf(stderr, "missing SPDX created timestamp\n");
    failures++;
  }
  if (file_contains_text("build/sbom-metadata.spdx.json", "1970-01-01T00:00:00Z")) {
    fprintf(stderr, "SPDX timestamp still epoch\n");
    failures++;
  }
  if (!file_contains_text("build/sbom-metadata.spdx.json", "https://tack.invalid/spdx/app/")) {
    fprintf(stderr, "missing SPDX namespace seed\n");
    failures++;
  }

  sv_free(&includes);
  sv_free(&defines);
  sv_free(&cflags);
  sv_free(&ldflags);
  sv_free(&libs);
  sv_free(&srcs);
  sv_free(&core_srcs);

  if (failures != 0) return 1;
  puts("sbom_metadata_test: ok");
  return 0;
}
