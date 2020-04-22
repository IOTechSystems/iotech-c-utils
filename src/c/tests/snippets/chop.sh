#!/bin/bash
csplit --suppress-matched --elide-empty-files --prefix=code snippets.c '/^\/\/ CUT$/' '{*}'
ls | grep "[02468]$" | xargs rm
files=$(ls code*)
i=0
for f in ${files}
do
  echo "<pre>" >> section_${i}
  cat ${f} >> section_${i}
  echo "</pre>" >> section_${i}
  rm ${f}
  ((i=i+1))
done
