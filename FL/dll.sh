for i in $(ls *.H) $(ls *.h); do
	sed -f dll.sed < $i > tmp
	mv tmp $i
done
