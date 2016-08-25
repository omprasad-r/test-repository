<?php

namespace Livefyre\Core;


class Core {
    const ENCRYPTION = "HS256";

    public static function getClassName() {
        return get_called_class();
    }
}