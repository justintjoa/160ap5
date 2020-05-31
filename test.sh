for file in ./tests/*.l1
do
    build/c1 "${file}" "${file}.trial"
done



