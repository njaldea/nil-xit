// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

import typescript from '@rollup/plugin-typescript';
import dts from 'rollup-plugin-dts'; // For bundling types

export default [
    {
        input: 'out_flatc/index.ts',
        output: [
            {
                file: 'dist/esm/bundle.esm.js',
                format: 'es'
            }
        ],
        plugins: [
            typescript({ tsconfig: './tsconfig.json' })
        ],
        external: ["flatbuffers"]
    },
    {
        input: 'out_flatc/index.ts',
        output: [
            {
                file: 'dist/types/bundle.d.ts',
                format: 'es'
            }
        ],
        plugins: [dts()],
        external: ["flatbuffers"]
    }
];