cmake . && make && ./main &&
for file in ./graphs/*.gv
do
  dot -Tsvg -o"$file".svg "$file"
done
