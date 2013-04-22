var atom = require('atom');
var Window = require('window');

var mainWindow = null;

process.on('message', function() {
  console.log.apply(this, arguments);
});

atom.browserMainParts.preMainMessageLoopRun = function() {
  mainWindow = new Window({ width: 800, height: 600 });
  mainWindow.url = 'file://' + __dirname + '/index.html';

  mainWindow.on('page-title-updated', function(event, title) {
    event.preventDefault();

    this.title = 'Atom Shell - ' + title;
  });
}