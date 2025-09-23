const App = () => {
  const sections = [1, 2, 3];

  return (
    <div>
      {sections.map((n, i) => (
        // 'key' est utilisé par react pour pister les items de la liste et leurs changements.
        // Chaque valeur 'key' doit être unique.
        <div key={"section-" + n}>
          Section {n} {i === 0 && <span>(first)</span>}
        </div>
      ))}
    </div>
  );
};
