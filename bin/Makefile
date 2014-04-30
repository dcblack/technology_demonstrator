#!make -f

.PHONY: help manifest status diff update force

MANIFEST:=MANIFEST.txt

help:
	echo "Targets: help manifest update"

manifest:
	@/bin/ls -1 | perl -lne 'print unless m{${MANIFEST}|Makefile}' >${MANIFEST}
	@echo "Created ${MANIFEST}

update:
	for f in `cat ${MANIFEST}`; do test -r $$HLDW/bin/$$f && rsync -auvP $$HLDW/bin/$$f ./; done

force:
	for f in `cat ${MANIFEST}`; do test -r $$HLDW/bin/$$f && rsync -auvP $$HLDW/bin/$$f ./; done

diff:
	for f in `cat ${MANIFEST}`; do test -r $$HLDW/bin/$$f && ${VISUAL} -d $$HLDW/bin/$$f ./$$f; done

status:
	for f in `cat ${MANIFEST}`; do test -r $$HLDW/bin/$$f && diff --brief $$HLDW/bin/$$f ./$$f; done

#EOF
