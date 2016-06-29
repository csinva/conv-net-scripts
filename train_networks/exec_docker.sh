raw_file=/groups/turaga/home/singhc/cremi/cremi-deform/test_data/input.h5
seg_file=/groups/turaga/home/singhc/cremi/cremi-deform/test_data/input.h5
out_file=/groups/turaga/home/singhc/cremi/cremi-deform/test_data/output.h5
EXEC_ARGS="-i1 $raw_file -i2 $seg_file -o $out_file"
MAIN="org.janelia.saalfeldlab.deform.Deform"
deform="env MAVEN_OPTS="-Xmx7g" mvn exec:java -Dexec.mainClass=\"$MAIN\" -Dexec.args=\"$EXEC_ARGS\""
eval $deform

code_dir=/groups/turaga/home/singhc/conv_net_scripts/train_networks
data_dir=/groups/turaga/home/singhc/conv_net_scripts/train_networks/rotations/
docker_data_dir=/data/SNEMI_aug
run_docker="sudo NV_GPU=2 nvidia-docker run -v $code_dir:/workspace -v $data_dir:$docker_data_dir:ro grisaitis/greentea python -u train.py"

sudo NV_GPU=2 nvidia-docker run -v /groups/turaga/home/singhc/conv_net_scripts/train_networks:/workspace -v /groups/turaga/home/singhc/conv_net_scripts/train_networks/rotations/:/data/SNEMI_aug:ro grisaitis/greentea python -u train.py