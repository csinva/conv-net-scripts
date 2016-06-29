raw_file=/groups/turaga/home/turagas/data/SNEMI3D/train_im_uint8.h5
seg_file=/groups/turaga/home/turagas/data/SNEMI3D/train_seg.h5
dset1=main
dset2=main
raw_out_file=/nobackup/turaga/singhc/SNEMI_aug/raw_out.h5
seg_out_file=/nobackup/turaga/singhc/SNEMI_aug/seg_out.h5
EXEC_ARGS="-i1 $raw_file -i2 $seg_file -d1 $dset1 -d2 $dset2 -o1 $raw_out_file -o2 $seg_out_file"
MAIN="org.janelia.saalfeldlab.deform.Deform"
deform="env MAVEN_OPTS="-Xmx7g" mvn exec:java -Dexec.mainClass=\"$MAIN\" -Dexec.args=\"$EXEC_ARGS\" -e"
eval $deform