code_dir=/groups/turaga/home/singhc/conv_net_scripts/train_networks
data_dir=/groups/turaga/home/singhc/conv_net_scripts/train_networks/rotations/
docker_data_dir=/data/SNEMI_aug
run_docker="sudo NV_GPU=2 nvidia-docker run -v $code_dir:/workspace -v $data_dir:$docker_data_dir:ro grisaitis/greentea python -u train.py"
eval $run_docker