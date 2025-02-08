echo Generating dependencies...

rm -f Makefile.deps

if [ "${OS}" == "Windows_NT" ]; then
	cmd="g++ -g -fPIC -municode -DNOSTDFILESYSTEM -DQUIET -O3 --std=c++17 -DNOIMPLOT -I./imgui -I./imgui/backends"
else
	cmd="g++ -g -fPIC -DQUIET -O3 --std=c++17 -DNOIMPLOT -I./imgui -I./imgui/backends"
fi

function add {
  a=$(${cmd} -MM $1)
  if [ $? -eq 0 ]
  then
    d=$(dirname $1)
    if [ "${d}" = "." ]
    then
      d=""
    else
      d="${d}/"
    fi
    echo "obj/${d}${a}" >> obj/Makefile.deps
    echo "	${cmd} -c $< -o \$@" >> obj/Makefile.deps
    echo "" >> obj/Makefile.deps
  fi
}

for i in "$@"
do
  echo gen for ${i}
  add ${i}
done
