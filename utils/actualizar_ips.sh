#!/bin/bash
#Para usar: 
#chmod +x actualizar_ips.sh
#./actualizar_ips.sh <memoria/kernel/cpu/entradasalida> <ip memoria> <ip cpu> <ip kernel>

# Verifica si se han proporcionado los argumentos necesarios
if [ $# -ne 4 ]; then
    echo "Uso: $0 <directorio> <nueva_ip_memoria> <nueva_ip_cpu> <nueva_ip_kernel>"
    exit 1
fi

# Asigna los parámetros a variables
DIRECTORY_PART="$1"
NEW_IP_MEMORIA="$2"
NEW_IP_CPU="$3"
NEW_IP_KERNEL="$4"

# Define el directorio base y el directorio específico
CONFIG_DIR="../${DIRECTORY_PART}/config_files"

# Verifica si el directorio existe
if [ ! -d "$CONFIG_DIR" ]; then
    echo "El directorio $CONFIG_DIR no existe"
    exit 1
fi

# Función para actualizar un archivo de configuración
update_config_file() {
    local config_file=$1
    local key=$2
    local new_value=$3

    if grep -q "^$key=" "$config_file"; then
        sed -i "s/^$key=.*/$key=$new_value/" "$config_file"
        echo "Actualizado $key en $config_file"
    else
        echo "$key no encontrado en $config_file"
    fi
}

# Actualiza los archivos de configuración en el directorio especificado
for config_file in "$CONFIG_DIR"/*.config; do
    update_config_file "$config_file" "IP_MEMORIA" "$NEW_IP_MEMORIA"
    update_config_file "$config_file" "IP_CPU" "$NEW_IP_CPU"
    update_config_file "$config_file" "IP_KERNEL" "$NEW_IP_KERNEL"
done

echo "Actualización completa."