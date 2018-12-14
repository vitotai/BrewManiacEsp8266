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
          src: './src/index.tmpl.html'
          dest: './dist/index.tmpl.html'
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
          'dist/index.tmpl.html': 'dist/index.tmpl.html'
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
          'build/index.tmpl.html': 'build/index.tmpl.html'
        }]

    comboall:
      main:
        files: [
            { 'dist/index.tmpl.html': ['build/index.tmpl.html'] }
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
          src: ['dist/english/*.htm', 'dist/spanish/*.htm', 'dist/russian/*.htm']
          dest: '.'
          ext: '.htm.gz'
        }]

    processhtml:
      dist:
        files: [
          'build/index.tmpl.html': ['src/index.tmpl.html']
        ]
  
    multi_lang_site_generator:
      default:
          options:
            vocabs:           ['english', 'spanish', 'russian']
            vocab_directory:  'src/locales'
            output_directory: 'dist'
            template_directory: 'dist'
          files: [
            'bm.htm': ['index.tmpl.html']
          ]


    watch:
      files: [
        'src/**/*'
      ]
      tasks: 'default'

  grunt.registerTask 'build', [
    #'jshint'
    'copy'
    'processhtml'
    'htmlmin:dev'
    'sass:dev'
    'postcss'
    'comboall'
    'htmlmin:dist'
    'compress'
  ]

  grunt.registerTask 'debug', [
    #'jshint'
    'copy'
    'processhtml'
    'htmlmin:dev'
    'comboall'
    'multi_lang_site_generator'
    'compress'
  ]

  grunt.registerTask 'default', [
    #'jshint'
    'copy'
    'processhtml'
    'htmlmin:dev'
    'sass:dev'
    'postcss'
    'watch'
  ]


  grunt.registerTask 'i18n', [
    'copy'
    'processhtml'
    'htmlmin:dev'
    'comboall'
    'htmlmin:dist'
    'multi_lang_site_generator'
    'compress'
  ]
