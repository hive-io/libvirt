#include <config.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef WITH_XEN

#include "internal.h"
#include "xml.h"
#include "xend_internal.h"
#include "testutils.h"

static char *progname;
static char *abs_srcdir;

#define MAX_FILE 4096

static int testCompareFiles(const char *xml, const char *sexpr,
                            int xendConfigVersion) {
  char xmlData[MAX_FILE];
  char sexprData[MAX_FILE];
  char *gotxml = NULL;
  char *xmlPtr = &(xmlData[0]);
  char *sexprPtr = &(sexprData[0]);
  int ret = -1;

  if (virtTestLoadFile(xml, &xmlPtr, MAX_FILE) < 0) {
      printf("Missing %s\n", xml);
      goto fail;
  }

  if (virtTestLoadFile(sexpr, &sexprPtr, MAX_FILE) < 0) {
      printf("Missing %s\n", sexpr);
      goto fail;
  }

  if (!(gotxml = xend_parse_domain_sexp(NULL, sexprData, xendConfigVersion)))
    goto fail;

  if (STRNEQ(xmlData, gotxml)) {
      virtTestDifference(stderr, xmlData, gotxml);
      goto fail;
  }

  ret = 0;

 fail:
  free(gotxml);

  return ret;
}

struct testInfo {
    const char *input;
    const char *output;
    int version;
};

static int testCompareHelper(const void *data) {
    const struct testInfo *info = data;
    char xml[PATH_MAX];
    char args[PATH_MAX];
    snprintf(xml, PATH_MAX, "%s/sexpr2xmldata/sexpr2xml-%s.xml",
             abs_srcdir, info->input);
    snprintf(args, PATH_MAX, "%s/sexpr2xmldata/sexpr2xml-%s.sexpr",
             abs_srcdir, info->output);
    return testCompareFiles(xml, args, info->version);
}


int
main(int argc, char **argv)
{
    int ret = 0;
    char cwd[PATH_MAX];

    progname = argv[0];

    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", progname);
        exit(EXIT_FAILURE);
    }

    abs_srcdir = getenv("abs_srcdir");
    if (!abs_srcdir)
        abs_srcdir = getcwd(cwd, sizeof(cwd));

    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", progname);
        exit(EXIT_FAILURE);
    }

#define DO_TEST(in, out, version)                                      \
    do {                                                               \
        struct testInfo info = { in, out, version };                   \
        if (virtTestRun("Xen SEXPR-2-XML " in " -> " out,              \
                        1, testCompareHelper, &info) < 0)              \
            ret = -1;                                                  \
    } while (0)

    DO_TEST("pv", "pv", 1);
    DO_TEST("fv", "fv", 1);
    DO_TEST("pv", "pv", 2);
    DO_TEST("fv-v2", "fv-v2", 2);
    DO_TEST("pv-vfb-orig", "pv-vfb-orig", 2);
    DO_TEST("pv-vfb-new", "pv-vfb-new", 3);
    DO_TEST("pv-bootloader", "pv-bootloader", 1);

    DO_TEST("disk-file", "disk-file", 2);
    DO_TEST("disk-block", "disk-block", 2);
    DO_TEST("disk-block-shareable", "disk-block-shareable", 2);
    DO_TEST("disk-drv-blktap-raw", "disk-drv-blktap-raw", 2);
    DO_TEST("disk-drv-blktap-qcow", "disk-drv-blktap-qcow", 2);

    DO_TEST("curmem", "curmem", 1);
    DO_TEST("net-routed", "net-routed", 2);
    DO_TEST("net-bridged", "net-bridged", 2);
    DO_TEST("net-e1000", "net-e1000", 2);
    DO_TEST("no-source-cdrom", "no-source-cdrom", 1);

    DO_TEST("fv-utc", "fv-utc", 1);
    DO_TEST("fv-localtime", "fv-localtime", 1);
    DO_TEST("fv-usbmouse", "fv-usbmouse", 1);
    DO_TEST("fv-usbmouse", "fv-usbmouse", 1);
    DO_TEST("fv-kernel", "fv-kernel", 1);

    DO_TEST("fv-serial-null", "fv-serial-null", 1);
    DO_TEST("fv-serial-file", "fv-serial-file", 1);
    DO_TEST("fv-serial-stdio", "fv-serial-stdio", 1);
    DO_TEST("fv-serial-pty", "fv-serial-pty", 1);
    DO_TEST("fv-serial-pipe", "fv-serial-pipe", 1);
    DO_TEST("fv-serial-tcp", "fv-serial-tcp", 1);
    DO_TEST("fv-serial-udp", "fv-serial-udp", 1);
    DO_TEST("fv-serial-tcp-telnet", "fv-serial-tcp-telnet", 1);
    DO_TEST("fv-serial-unix", "fv-serial-unix", 1);
    DO_TEST("fv-parallel-tcp", "fv-parallel-tcp", 1);

    DO_TEST("fv-sound", "fv-sound", 1);
    DO_TEST("fv-sound-all", "fv-sound-all", 1);

    exit(ret==0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
#else /* WITHOUT_XEN */
int
main(void)
{
    fprintf(stderr, "libvirt compiled without Xen support\n");
    return(0);
}
#endif /* WITH_XEN */
