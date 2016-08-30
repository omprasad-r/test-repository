<?php

namespace Livefyre\Routing;


use Livefyre\Exceptions\ApiException;

/**
 * @codeCoverageIgnore
 */
class WPClient {
    public static function get($url, $headers) {
        $response = wp_remote_get($url, array("headers" => $headers));

        self::examineResponse($response["response"]["code"]);
        return $response["body"];
    }

    public static function post($url, $headers, $data) {
        $response = wp_remote_post($url, array("headers"=>$headers, "body"=>$data));

        self::examineResponse($response["response"]["code"]);
        return $response["body"];
    }

    public static function put($url, $headers, $data) {
        $response = wp_remote_request($url, array("headers"=>$headers, "body"=>$data));

        self::examineResponse($response["response"]["code"]);
        return $response["body"];
    }

    public static function patch($url, $headers, $data) {
        $response = wp_remote_request($url, array("headers"=>$headers, "body"=>$data));

        self::examineResponse($response["response"]["code"]);
        return $response["body"];
    }

    private static function examineResponse($code) {
        if ($code >= 400) {
            throw new ApiException($code);
        }
    }
}