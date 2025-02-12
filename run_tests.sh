#!/bin/bash


# Sets a list of the number of parties
party_counts=(2 4 8 16)
base_port=23000
input_values=(5 50 500)
ip_address="127.0.0.1"

# Setting the network bandwidth
# # LAN / Basic / ReLU
./throttle.sh LAN

# Iterate over the number of different parties
for number_parties in "${party_counts[@]}"
do
    for input_value in "${input_values[@]}"
    do
        log_dir="./logs/lan/basic/relu/mp${number_parties}_input${input_value}/"  # Dynamically generate log directories based on number_parties
        mkdir -p $log_dir  # Create log directory

        width_value=$((input_value / 5))

        # Build --parties parameters
        parties=""
        for (( i=0; i<number_parties; i++ ))
        do
            if [ $i -ne 0 ]; then
                parties+=" "  
            fi
            parties+="$i,$ip_address,$((base_port + i))"
        done

        # Start each participant and record the output and error logs separately
        for (( i=0; i<number_parties; i++ ))
        do
            log_file="$log_dir/party_$i.log"
            echo "Executing: bin/test_alexnet --my-id $i --parties $parties --input $input_value" | tee -a $log_file
            bin/test_alexnet_bR --my-id $i --parties $parties --input $input_value --height 5 --width $width_value >> $log_file 2>&1 &
        done

        # Print completion log
        echo "All processes for number_parties=$number_parties have been started. Check logs in $log_dir"

        # Wait for all background processes to complete
        wait

        
        echo "Waiting for processes to finish. Sleeping for 60 seconds..."
        sleep 60  # Pause for 60 seconds or adjust the time as needed
    done
done


# Setting the network bandwidth
# # LAN / Basic / Maxpool
./throttle.sh LAN

# Iterate over the number of different parties
for number_parties in "${party_counts[@]}"
do
    for input_value in "${input_values[@]}"
    do
        log_dir="./logs/lan/basic/maxpool/mp${number_parties}_input${input_value}/"  # Dynamically generate log directories based on number_parties
        mkdir -p $log_dir  # Create log directory

        width_value=$((input_value / 5))

        # Build --parties parameters
        parties=""
        for (( i=0; i<number_parties; i++ ))
        do
            if [ $i -ne 0 ]; then
                parties+=" "  
            fi
            parties+="$i,$ip_address,$((base_port + i))"
        done

        # Start each participant and record the output and error logs separately
        for (( i=0; i<number_parties; i++ ))
        do
            log_file="$log_dir/party_$i.log"
            echo "Executing: bin/test_alexnet --my-id $i --parties $parties --input $input_value" | tee -a $log_file
            bin/test_alexnet_bM --my-id $i --parties $parties --input $input_value --height 5 --width $width_value >> $log_file 2>&1 &
        done

        # Print completion log
        echo "All processes for number_parties=$number_parties have been started. Check logs in $log_dir"

        # Wait for all background processes to complete
        wait

        
        echo "Waiting for processes to finish. Sleeping for 60 seconds..."
        sleep 60  # Pause for 60 seconds or adjust the time as needed
    done
done


# Setting the network bandwidth
# # WAN / Basic / ReLU
./throttle.sh WAN

# Iterate over the number of different parties
for number_parties in "${party_counts[@]}"
do
    for input_value in "${input_values[@]}"
    do
        log_dir="./logs/wan/basic/relu/mp${number_parties}_input${input_value}/"  # Dynamically generate log directories based on number_parties
        mkdir -p $log_dir  # Create log directory

        width_value=$((input_value / 5))

        # Build --parties parameters
        parties=""
        for (( i=0; i<number_parties; i++ ))
        do
            if [ $i -ne 0 ]; then
                parties+=" "  
            fi
            parties+="$i,$ip_address,$((base_port + i))"
        done

        # Start each participant and record the output and error logs separately
        for (( i=0; i<number_parties; i++ ))
        do
            log_file="$log_dir/party_$i.log"
            echo "Executing: bin/test_alexnet --my-id $i --parties $parties --input $input_value" | tee -a $log_file
            bin/test_alexnet_bR --my-id $i --parties $parties --input $input_value --height 5 --width $width_value >> $log_file 2>&1 &
        done

        # Print completion log
        echo "All processes for number_parties=$number_parties have been started. Check logs in $log_dir"

        # Wait for all background processes to complete
        wait

        
        echo "Waiting for processes to finish. Sleeping for 60 seconds..."
        sleep 60  # Pause for 60 seconds or adjust the time as needed
    done
done
# Setting the network bandwidth
# # WAN / Basic / Maxpool
./throttle.sh WAN

# Iterate over the number of different parties
for number_parties in "${party_counts[@]}"
do
    for input_value in "${input_values[@]}"
    do
        log_dir="./logs/wan/basic/maxpool/mp${number_parties}_input${input_value}/"  # Dynamically generate log directories based on number_parties
        mkdir -p $log_dir  # Create log directory

        width_value=$((input_value / 5))

        # Build --parties parameters
        parties=""
        for (( i=0; i<number_parties; i++ ))
        do
            if [ $i -ne 0 ]; then
                parties+=" "  
            fi
            parties+="$i,$ip_address,$((base_port + i))"
        done

        # Start each participant and record the output and error logs separately
        for (( i=0; i<number_parties; i++ ))
        do
            log_file="$log_dir/party_$i.log"
            echo "Executing: bin/test_alexnet --my-id $i --parties $parties --input $input_value" | tee -a $log_file
            bin/test_alexnet_bM --my-id $i --parties $parties --input $input_value --height 5 --width $width_value >> $log_file 2>&1 &
        done

        # Print completion log
        echo "All processes for number_parties=$number_parties have been started. Check logs in $log_dir"

        # Wait for all background processes to complete
        wait

        
        echo "Waiting for processes to finish. Sleeping for 60 seconds..."
        sleep 60  # Pause for 60 seconds or adjust the time as needed
    done
done

# Setting the network bandwidth
# # LAN / Optimised / ReLU
./throttle.sh LAN

# Iterate over the number of different parties
for number_parties in "${party_counts[@]}"
do
    for input_value in "${input_values[@]}"
    do
        log_dir="./logs/lan/optimised/relu/mp${number_parties}_input${input_value}/"  # Dynamically generate log directories based on number_parties
        mkdir -p $log_dir  # Create log directory

        width_value=$((input_value / 5))

        # Build --parties parameters
        parties=""
        for (( i=0; i<number_parties; i++ ))
        do
            if [ $i -ne 0 ]; then
                parties+=" "  
            fi
            parties+="$i,$ip_address,$((base_port + i))"
        done

        # Start each participant and record the output and error logs separately
        for (( i=0; i<number_parties; i++ ))
        do
            log_file="$log_dir/party_$i.log"
            echo "Executing: bin/test_alexnet --my-id $i --parties $parties --input $input_value" | tee -a $log_file
            bin/test_alexnet_oR --my-id $i --parties $parties --input $input_value --height 5 --width $width_value >> $log_file 2>&1 &
        done

        # Print completion log
        echo "All processes for number_parties=$number_parties have been started. Check logs in $log_dir"

        # Wait for all background processes to complete
        wait

        
        echo "Waiting for processes to finish. Sleeping for 60 seconds..."
        sleep 60  # Pause for 60 seconds or adjust the time as needed
    done
done


# Setting the network bandwidth
# # LAN / Optimised / Maxpool
./throttle.sh LAN

# Iterate over the number of different parties
for number_parties in "${party_counts[@]}"
do
    for input_value in "${input_values[@]}"
    do
        log_dir="./logs/lan/optimised/maxpool/mp${number_parties}_input${input_value}/"  # Dynamically generate log directories based on number_parties
        mkdir -p $log_dir  # Create log directory

        width_value=$((input_value / 5))

        # Build --parties parameters
        parties=""
        for (( i=0; i<number_parties; i++ ))
        do
            if [ $i -ne 0 ]; then
                parties+=" "  
            fi
            parties+="$i,$ip_address,$((base_port + i))"
        done

        # Start each participant and record the output and error logs separately
        for (( i=0; i<number_parties; i++ ))
        do
            log_file="$log_dir/party_$i.log"
            echo "Executing: bin/test_alexnet --my-id $i --parties $parties --input $input_value" | tee -a $log_file
            bin/test_alexnet_oM --my-id $i --parties $parties --input $input_value --height 5 --width $width_value >> $log_file 2>&1 &
        done

        # Print completion log
        echo "All processes for number_parties=$number_parties have been started. Check logs in $log_dir"

        # Wait for all background processes to complete
        wait

        
        echo "Waiting for processes to finish. Sleeping for 60 seconds..."
        sleep 60  # Pause for 60 seconds or adjust the time as needed
    done
done


# Setting the network bandwidth
# # WAN / Optimised / ReLU
./throttle.sh WAN

# Iterate over the number of different parties
for number_parties in "${party_counts[@]}"
do
    for input_value in "${input_values[@]}"
    do
        log_dir="./logs/wan/optimised/relu/mp${number_parties}_input${input_value}/"  # Dynamically generate log directories based on number_parties
        mkdir -p $log_dir  # Create log directory

        width_value=$((input_value / 5))

        # Build --parties parameters
        parties=""
        for (( i=0; i<number_parties; i++ ))
        do
            if [ $i -ne 0 ]; then
                parties+=" "  
            fi
            parties+="$i,$ip_address,$((base_port + i))"
        done

        # Start each participant and record the output and error logs separately
        for (( i=0; i<number_parties; i++ ))
        do
            log_file="$log_dir/party_$i.log"
            echo "Executing: bin/test_alexnet --my-id $i --parties $parties --input $input_value" | tee -a $log_file
            bin/test_alexnet_oR --my-id $i --parties $parties --input $input_value --height 5 --width $width_value >> $log_file 2>&1 &
        done

        # Print completion log
        echo "All processes for number_parties=$number_parties have been started. Check logs in $log_dir"

        # Wait for all background processes to complete
        wait

        
        echo "Waiting for processes to finish. Sleeping for 60 seconds..."
        sleep 60  # Pause for 60 seconds or adjust the time as needed
    done
done
# Setting the network bandwidth
# # WAN / Optimised / Maxpool
./throttle.sh WAN

# Iterate over the number of different parties
for number_parties in "${party_counts[@]}"
do
    for input_value in "${input_values[@]}"
    do
        log_dir="./logs/wan/optimised/maxpool/mp${number_parties}_input${input_value}/"  # Dynamically generate log directories based on number_parties
        mkdir -p $log_dir  # Create log directory

        width_value=$((input_value / 5))

        # Build --parties parameters
        parties=""
        for (( i=0; i<number_parties; i++ ))
        do
            if [ $i -ne 0 ]; then
                parties+=" "  
            fi
            parties+="$i,$ip_address,$((base_port + i))"
        done

        # Start each participant and record the output and error logs separately
        for (( i=0; i<number_parties; i++ ))
        do
            log_file="$log_dir/party_$i.log"
            echo "Executing: bin/test_alexnet --my-id $i --parties $parties --input $input_value" | tee -a $log_file
            bin/test_alexnet_oM --my-id $i --parties $parties --input $input_value --height 5 --width $width_value >> $log_file 2>&1 &
        done

        # Print completion log
        echo "All processes for number_parties=$number_parties have been started. Check logs in $log_dir"

        # Wait for all background processes to complete
        wait

        
        echo "Waiting for processes to finish. Sleeping for 60 seconds..."
        sleep 60  # Pause for 60 seconds or adjust the time as needed
    done
done


