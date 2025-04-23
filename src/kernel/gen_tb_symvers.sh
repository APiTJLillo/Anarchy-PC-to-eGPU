#!/bin/bash

# Create Module.symvers.thunderbolt with required symbols
cat > Module.symvers.thunderbolt << EOF
0xf7370f56	tb_bus_type	drivers/thunderbolt/thunderbolt	EXPORT_SYMBOL_GPL
0xbea5ff1e	tb_service_register_handler	drivers/thunderbolt/thunderbolt	EXPORT_SYMBOL_GPL
0x12345678	tb_service_unregister_handler	drivers/thunderbolt/thunderbolt	EXPORT_SYMBOL_GPL
EOF

# Only copy system Module.symvers if it doesn't exist yet
if [ ! -f Module.symvers ]; then
    cp /lib/modules/$(uname -r)/build/Module.symvers Module.symvers
fi

# Append our symbols, avoiding duplicates
cat Module.symvers.thunderbolt >> Module.symvers