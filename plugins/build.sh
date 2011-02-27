CPP="g++"
if [ "$TARGET" == "fritzbox" -o "$TARGET" == "fb" ]; then
  TARGET="mipsel-linux-uclibc"
fi

if [ -n "$TARGET" ]; then 
  CPP="$TARGET-$CPP";
fi

fails=""
for i in $@; do
  echo "building $i";
  if [ -e "$i.additional" ]; then
    add=$(cat "$i.additional")
  else
    add=""
  fi
  $CPP $i ../myfuncs.cpp $add -shared -o "${i%.*}".so || fails="$fails$i "
done

echo

if [ -n "$fails" ]; then
  for i in $fails; do
    echo "building $i failed!"
  done
else
  echo "all done!"
fi

echo
