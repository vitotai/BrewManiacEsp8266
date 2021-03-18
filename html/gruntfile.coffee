'use strict'

module.exports = (grunt) ->
  grunt.loadNpmTasks 'grunt-combo-html-css-js'
  grunt.loadNpmTasks 'grunt-contrib-compress'
  grunt.loadNpmTasks 'grunt-contrib-copy'
  grunt.loadNpmTasks 'grunt-contrib-htmlmin'
  grunt.loadNpmTasks 'grunt-contrib-jshint'
  grunt.loadNpmTasks 'grunt-contrib-sass'
  grunt.loadNpmTasks 'grunt-contrib-uglify'
  grunt.loadNpmTasks 'grunt-contrib-watch'
  grunt.loadNpmTasks 'grunt-postcss'
  grunt.loadNpmTasks 'grunt-processhtml'
  grunt.loadNpmTasks 'grunt-multi-lang-site-generator'

  grunt.initConfig

    copy:
      jsfiles:
        files: [{
          expand: true
          cwd: './src'
          src: './src/bm.html'
          dest: './build/bm.tmpl.html'
        },{
          expand: true
          cwd: './src/css'
          src: '**/*.css'
          dest: './build/css/'
        },
        {
          expand: true
          cwd: './src/js'
          src: '**/*.js'
          dest: './build/js/'
        },
         {
          expand: true
          cwd: './src/lib/bootstrap-4.4.1/css'
          src: '**/*.css'
          dest: './build/lib/bootstrap-4.4.1/css/'
        },
        {
          expand: true
          cwd: './src/lib/bootstrap-4.4.1/js'
          src: '**/*.js'
          dest: './build/lib/bootstrap-4.4.1/js/'
        },        
        {
          expand: true
          cwd: './src/lib/dygraph'
          src: '**/*.*'
          dest: './build/lib/dygraph/'
        },
        {
          expand: true
          cwd: './src/lib/jquery'
          src: '**/*.js'
          dest: './build/lib/jquery/'
        }]

    htmlmin:
      dist:
        options:
          removeComments: true,
          collapseWhitespace: true,
          collapseBooleanAttributes: true,
          removeAttributeQuotes: true,
          removeRedundantAttributes: false,
          removeEmptyAttributes: true,
          minifyJS: true,
          minifyCSS: true
        files: [{
          'dist/bm.tmpl.html': 'dist/bm.tmpl.html'
        }]
      dev:
        options:
          removeComments: false,
          collapseWhitespace: false,
          collapseBooleanAttributes: false,
          removeAttributeQuotes: false,
          removeRedundantAttributes: false,
          removeEmptyAttributes: false,
          minifyJS: false,
          minifyCSS: false
        files: [{
          'dist/bm.tmpl.html': 'dist/bm.tmpl.html'
        }]

    comboall:
      main:
        files: [
            { 'dist/bm.tmpl.html': ['build/bm.tmpl.html'] }
        ]

    jshint:
      # In case there is a /dist/ directory, we don't want to lint that
      # so we use the ! (bang) operator to ignore the specified directory
      files: [
        './*.js'
        '!dist/**'
      ]
      options:
        curly: true
        eqeqeq: true
        immed: true
        latedef: true
        newcap: true
        noarg: true
        sub: true
        undef: true
        boss: true
        eqnull: true
        browser: true
        globals:
          console: true

    sass:
      dev:
        options:
          style: 'expanded'
        expand: true
        cwd: 'src/styles/'
        src: [ '*.scss' ]
        dest: './build/'
        ext: '.css'

    postcss:
      options:
        map: true
        processors: [ require('autoprefixer') ]
      dist:
        src: 'build/*.css'

    compress:
      main:
        options:
          mode: 'gzip'
          level: 9
        expand: true
        files: [{
          expand: true
          src: ['dist/english/*.htm', 'dist/spanish/*.htm', 'dist/russian/*.htm','dist/portuguese-br/*.htm','dist/italiano/*.htm']
          dest: '.'
          ext: '.htm.gz'
        }]

    processhtml:
      dist:
        files: [
          'build/bm.tmpl.html': ['src/bm.html']
        ]
  
    multi_lang_site_generator:
      default:
          options:
            vocabs:           ['english', 'spanish', 'russian','portuguese-br','italiano']
            vocab_directory:  'src/locales'
            output_directory: 'dist'
            template_directory: 'dist'
          files: [
            'bm.htm': ['bm.tmpl.html']
          ]


    watch:
      files: [
        'src/**/*'
      ]
      tasks: 'default'


  grunt.registerTask 'debug', [
    #'jshint'
    'copy'
    'processhtml'
    'htmlmin:dev'
    'comboall'
    'multi_lang_site_generator'
    'compress'
  ]

  grunt.registerTask 'build', [
    'copy'
    'processhtml'
    'comboall'
    'htmlmin:dist'
    'multi_lang_site_generator'
    'compress'
  ]
