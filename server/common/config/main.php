<?php
return [
    'name' => 'guard',
    'vendorPath' => dirname(dirname(__DIR__)) . '/vendor',
    'components' => [
        'cache' => [
            'class' => 'yii\redis\Cache',
            'redis' => 'redis',
        ],
    ],
    'modules' => [
    ],
];
