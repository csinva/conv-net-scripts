raw_file=/groups/turaga/home/singhc/cremi/cremi-deform/test_data/input.h5
seg_file=/groups/turaga/home/singhc/cremi/cremi-deform/test_data/input.h5
out_file=/groups/turaga/home/singhc/cremi/cremi-deform/test_data/output.h5
EXEC_ARGS="-i1 $raw_file -i2 $seg_file -o $out_file"
MAIN="org.janelia.saalfeldlab.deform.Deform"
deform="env MAVEN_OPTS="-Xmx7g" mvn exec:java -Dexec.mainClass=\"$MAIN\" -Dexec.args=\"$EXEC_ARGS\""
eval $deform